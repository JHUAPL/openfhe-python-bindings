// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

// encrypt, decrypt, keygeneration, and the like

#include <complex>
#include <stdexcept>
#include <vector>

// string formatting for exceptions
#include <fmt/format.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "bgv/BGV_ciphertext_extension.hpp"
#include "bgv/BGV_key_operations.hpp"
#include "utils/utils.hpp"

using namespace boost::python;
using namespace boost::python::numpy;
using namespace lbcrypto;
using namespace pyOpenFHE;

namespace pyOpenFHE_BGV {

BGVCryptoContext genBGVContext(usint multiplicativeDepth, usint batchSize,
                               usint plaintextModulus, SecurityLevel stdLevel,
                               usint ringDim) {
  CCParams<CryptoContextBGVRNS> parameters;
  parameters.SetMultiplicativeDepth(multiplicativeDepth);
  parameters.SetBatchSize(batchSize);
  parameters.SetPlaintextModulus(plaintextModulus);
  parameters.SetSecurityLevel(stdLevel);
  parameters.SetRingDim(ringDim);

  // trying this out, this may fix ModRescale
  // parameters.SetScalingTechnique(FIXEDMANUAL);

  CryptoContext<DCRTPoly> native_cc = GenCryptoContext(parameters);

  // convert to BGVCryptoContext
  BGVCryptoContext cc = BGVCryptoContext(native_cc);
  std::cout << "BGV scheme is using ring dimension = " << cc.getRingDimension()
            << std::endl;
  std::cout << "batch size = " << cc.getBatchSize() << std::endl;
  std::cout << "plaintext modulus = " << cc.getPlaintextModulus() << std::endl;

  std::cout << std::endl;

  return cc;
}

// Encode a C++ vector into an OpenFHE Plaintext object
Plaintext BGVCryptoContext::encode(std::vector<int64_t> vals) {
  if (vals.size() != context->GetEncodingParams()->GetBatchSize()) {
    std::string s =
        fmt::format("Provided vector has length = {}, but the CryptoContext "
                    "batch size = {}",
                    vals.size(), context->GetEncodingParams()->GetBatchSize());
    throw std::runtime_error(s);
  }
  // cyclically duplicate the input
  size_t final_size = context->GetRingDimension() / 2;
  tileVector(vals, final_size);
  // vanilla MakePackedPlaintext just takes an int64 vector
  Plaintext ptxt = context->MakePackedPlaintext(vals);
  return ptxt;
}

// Encrypt with: private key and python list
pyOpenFHE_BGV::BGVCiphertext
BGVCryptoContext::encryptPrivate(const PrivateKey<DCRTPoly> &privateKey,
                                 const list &pyvals) {
  std::vector<int64_t> vals = pyOpenFHE::pythonListToCppLongIntVector(pyvals);
  auto ptxt = encode(vals);
  return pyOpenFHE_BGV::BGVCiphertext(context->Encrypt(privateKey, ptxt));
}

// public key and python list
pyOpenFHE_BGV::BGVCiphertext
BGVCryptoContext::encryptPublic(const PublicKey<DCRTPoly> &publicKey,
                                const list &pyvals) {
  std::vector<int64_t> vals = pyOpenFHE::pythonListToCppLongIntVector(pyvals);
  auto ptxt = encode(vals);
  return pyOpenFHE_BGV::BGVCiphertext(context->Encrypt(publicKey, ptxt));
}

// private key and numpy array
pyOpenFHE_BGV::BGVCiphertext
BGVCryptoContext::encryptPrivate2(const PrivateKey<DCRTPoly> &privateKey,
                                  const ndarray &pyvals) {
  std::vector<int64_t> vals = pyOpenFHE::numpyListToCppLongIntVector(pyvals);
  auto ptxt = encode(vals);
  return pyOpenFHE_BGV::BGVCiphertext(context->Encrypt(privateKey, ptxt));
}

// public key and numpy array
pyOpenFHE_BGV::BGVCiphertext
BGVCryptoContext::encryptPublic2(const PublicKey<DCRTPoly> &publicKey,
                                 const ndarray &pyvals) {
  std::vector<int64_t> vals = pyOpenFHE::numpyListToCppLongIntVector(pyvals);
  //  std::cout << "hello 1" << std::endl;
  //  std::cout << "vals size " << vals.size() << std::endl;
  //  std::cout << "slots " << self.size << std::endl;
  auto ptxt = encode(vals);
  //  std::cout << "hello 2" << std::endl;
  return pyOpenFHE_BGV::BGVCiphertext(context->Encrypt(publicKey, ptxt));
}

ndarray BGVCryptoContext::zeroPadToBatchSize(std::vector<int64_t> vals) {
  size_t batch_size = context->GetEncodingParams()->GetBatchSize();
  if (vals.size() > batch_size) {
    std::string s =
        fmt::format("Provided vector has length = {}, but the CryptoContext "
                    "batch size = {}",
                    vals.size(), context->GetEncodingParams()->GetBatchSize());
    throw std::runtime_error(s);
  }
  vals.resize(batch_size, 0);
  return pyOpenFHE::cppLongIntVectorToNumpyList(vals);
}

// python list
ndarray BGVCryptoContext::zeroPadToBatchSizeList(const list &pyvals) {
  std::vector<int64_t> vals = pyOpenFHE::pythonListToCppLongIntVector(pyvals);
  return zeroPadToBatchSize(vals);
}

// numpy array
ndarray BGVCryptoContext::zeroPadToBatchSizeNumpy(const ndarray &pyvals) {
  std::vector<int64_t> vals = pyOpenFHE::numpyListToCppLongIntVector(pyvals);
  return zeroPadToBatchSize(vals);
}

// again, usually decrypt takes a key, Ciphertext, and Plaintext reference
// and fills in the plaintext ref and returns whether or not the decryption was
// valid I want to give a key, BGVCiphertext wrapper, and receive a numpy array
// this does nothing with the return value of decrypt, but maybe there could be
// an error check here...
ndarray BGVCryptoContext::decrypt(const PrivateKey<DCRTPoly> &privateKey,
                                  pyOpenFHE_BGV::BGVCiphertext &ctxt) {
  Plaintext ptxt;
  // level reduce to level2 before decrypting
  auto algo = ctxt.cipher->GetCryptoContext()->GetScheme();
  auto ctxt2 = algo->Compress(ctxt.cipher, 2);
  context->Decrypt(privateKey, ctxt2, &ptxt);
  ptxt->SetLength(ctxt.cipher->GetEncodingParameters()->GetBatchSize());
  // we can skip the weird copy step, there could be an optimization here...
  auto vals = ptxt->GetPackedValue();
  return pyOpenFHE::cppLongIntVectorToNumpyList(vals);
}

/*
BGV Bootstrapping functions
*/
void BGVCryptoContext::evalBootstrapSetup() {
  std::vector<uint32_t> bsgsDim = {0, 0};
  std::vector<uint32_t> levelBudget = {4, 4};

  usint slots = context->GetEncodingParams()->GetBatchSize();
  context->EvalBootstrapSetup(levelBudget, bsgsDim, slots);
}

void BGVCryptoContext::evalBootstrapKeyGen(
    const PrivateKey<DCRTPoly> &privateKey) {
  usint slots = context->GetEncodingParams()->GetBatchSize();
  // usint n = self.GetRingDimension();

  context->EvalBootstrapKeyGen(privateKey, slots);
}

boost::python::list
BGVCryptoContext::evalBootstrapList(boost::python::list ctxts) {

  std::vector<pyOpenFHE_BGV::BGVCiphertext> input_ctxts(len(ctxts));
  auto output_ctxts = pyOpenFHE::make_list(len(ctxts));

#pragma omp parallel for
  for (int i = 0; i < len(ctxts); ++i) {
    input_ctxts[i] = extract<pyOpenFHE_BGV::BGVCiphertext>(ctxts[i]);
    input_ctxts[i].cipher = context->EvalBootstrap(input_ctxts[i].cipher);
    output_ctxts[i] = input_ctxts[i];
  }

  return output_ctxts;
}

pyOpenFHE_BGV::BGVCiphertext
BGVCryptoContext::evalBootstrap(pyOpenFHE_BGV::BGVCiphertext ctxt) {
  ctxt.cipher = context->EvalBootstrap(ctxt.cipher);
  return ctxt;
}

// make rotation keys for all of the +/- powers-of-2
// we should probably try and put all the scheme-agnostic functions somewhere
// neutral reduce code duplication and C++ won't complain about it if we ever
// link these libraries
// TODO: that ^
void BGVCryptoContext::evalPowerOf2RotationKeyGen(
    const PrivateKey<DCRTPoly> &privateKey) {
  int N = context->GetEncodingParams()->GetBatchSize();
  int M = context->GetRingDimension();
  N = std::min(N, M / 2);
  std::vector<int> index_list;
  int r = 1;
  while (r <= N) {
    index_list.push_back(r);
    index_list.push_back(-r);
    r *= 2;
  }
  context->EvalAtIndexKeyGen(privateKey, index_list, nullptr);
}

} // namespace pyOpenFHE_BGV
