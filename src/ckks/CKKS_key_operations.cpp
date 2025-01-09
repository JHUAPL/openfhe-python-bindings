// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

// encrypt, decrypt, keygeneration, and the like

#include <complex>
#include <stdexcept>
#include <vector>

// string formatting for exceptions
#include <fmt/format.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "ckks/CKKS_ciphertext_extension.hpp"
#include "ckks/CKKS_key_operations.hpp"
#include "ckks/serialization.hpp"
#include "utils/utils.hpp"

using namespace boost::python;
using namespace boost::python::numpy;
using namespace lbcrypto;
using namespace pyOpenFHE;

namespace pyOpenFHE_CKKS {

CKKSCryptoContext genCKKSContext(usint multiplicativeDepth,
                                 usint scalingFactorBits, usint batchSize,
                                 SecurityLevel stdLevel, usint ringDim) {
  CCParams<CryptoContextCKKSRNS> parameters;
  parameters.SetMultiplicativeDepth(multiplicativeDepth);
  parameters.SetScalingModSize(scalingFactorBits);
  parameters.SetBatchSize(batchSize);
  parameters.SetSecurityLevel(stdLevel);
  if(ringDim != 0) {
    parameters.SetRingDim(ringDim);
  }

  CryptoContext<DCRTPoly> native_cc = GenCryptoContext(parameters);
  
  // convert to CKKSCryptoContext
  CKKSCryptoContext cc = CKKSCryptoContext(native_cc);


  return cc;
}

// Encode a C++ vector into an OpenFHE Plaintext object
Plaintext CKKSCryptoContext::encode(std::vector<double> vals) {
  if (vals.size() != context->GetEncodingParams()->GetBatchSize()) {
    std::string s =
        fmt::format("Provided vector has length = {}, but the CryptoContext "
                    "batch size = {}",
                    vals.size(), context->GetEncodingParams()->GetBatchSize());
    throw std::runtime_error(s);
  }

  std::vector<std::complex<double>> cvals(vals.size());
  for (unsigned int i = 0; i < cvals.size(); i++) {
    cvals[i] = vals[i];
  }

  Plaintext ptxt = context->MakeCKKSPackedPlaintext(cvals);
  return ptxt;
}

// Encrypt with: private key and python list
pyOpenFHE_CKKS::CKKSCiphertext
CKKSCryptoContext::encryptPrivate(const PrivateKey<DCRTPoly> &privateKey,
                                  const list &pyvals) {
  std::vector<double> vals = pyOpenFHE::pythonListToCppDoubleVector(pyvals);
  auto ptxt = encode(vals);
  return pyOpenFHE_CKKS::CKKSCiphertext(context->Encrypt(privateKey, ptxt));
}

// public key and python list
pyOpenFHE_CKKS::CKKSCiphertext
CKKSCryptoContext::encryptPublic(const PublicKey<DCRTPoly> &publicKey,
                                 const list &pyvals) {
  std::vector<double> vals = pyOpenFHE::pythonListToCppDoubleVector(pyvals);
  auto ptxt = encode(vals);
  return pyOpenFHE_CKKS::CKKSCiphertext(context->Encrypt(publicKey, ptxt));
}

// private key and numpy array
pyOpenFHE_CKKS::CKKSCiphertext
CKKSCryptoContext::encryptPrivate2(const PrivateKey<DCRTPoly> &privateKey,
                                   const ndarray &pyvals) {
  std::vector<double> vals = pyOpenFHE::numpyListToCppDoubleVector(pyvals);
  auto ptxt = encode(vals);
  return pyOpenFHE_CKKS::CKKSCiphertext(context->Encrypt(privateKey, ptxt));
}

// public key and numpy array
pyOpenFHE_CKKS::CKKSCiphertext
CKKSCryptoContext::encryptPublic2(const PublicKey<DCRTPoly> &publicKey,
                                  const ndarray &pyvals) {
  std::vector<double> vals = pyOpenFHE::numpyListToCppDoubleVector(pyvals);
  //  std::cout << "hello 1" << std::endl;
  //  std::cout << "vals size " << vals.size() << std::endl;
  //  std::cout << "slots " << self.size << std::endl;
  auto ptxt = encode(vals);
  //  std::cout << "hello 2" << std::endl;
  return pyOpenFHE_CKKS::CKKSCiphertext(context->Encrypt(publicKey, ptxt));
}

ndarray CKKSCryptoContext::zeroPadToBatchSize(std::vector<double> vals) {
  size_t batch_size = context->GetEncodingParams()->GetBatchSize();
  if (vals.size() > batch_size) {
    std::string s =
        fmt::format("Provided vector has length = {}, but the CryptoContext "
                    "batch size = {}",
                    vals.size(), context->GetEncodingParams()->GetBatchSize());
    throw std::runtime_error(s);
  }
  vals.resize(batch_size, 0);
  return pyOpenFHE::cppDoubleVectorToNumpyList(vals);
}

// python list
ndarray CKKSCryptoContext::zeroPadToBatchSizeList(const list &pyvals) {
  std::vector<double> vals = pyOpenFHE::pythonListToCppDoubleVector(pyvals);
  return zeroPadToBatchSize(vals);
}

// numpy array
ndarray CKKSCryptoContext::zeroPadToBatchSizeNumpy(const ndarray &pyvals) {
  std::vector<double> vals = pyOpenFHE::numpyListToCppDoubleVector(pyvals);
  return zeroPadToBatchSize(vals);
}

// again, usually decrypt takes a key, Ciphertext, and Plaintext reference
// and fills in the plaintext ref and returns whether or not the decryption was
// valid I want to give a key, CKKSCiphertext wrapper, and receive a numpy array
// this does nothing with the return value of decrypt, but maybe there could be
// an error check here...
ndarray CKKSCryptoContext::decrypt(const PrivateKey<DCRTPoly> &privateKey,
                                   pyOpenFHE_CKKS::CKKSCiphertext &ctxt) {
  Plaintext ptxt;
  // level reduce to level2 before decrypting
  auto algo = ctxt.cipher->GetCryptoContext()->GetScheme();
  auto ctxt2 = algo->Compress(ctxt.cipher, 2);
  context->Decrypt(privateKey, ctxt2, &ptxt);
  ptxt->SetLength(ctxt.cipher->GetEncodingParameters()->GetBatchSize());
  auto cvals = ptxt->GetRealPackedValue();
  std::vector<double> vals(cvals.size());
  for (unsigned int i = 0; i < cvals.size(); i++) {
    vals[i] = std::real(cvals[i]);
  }
  return pyOpenFHE::cppDoubleVectorToNumpyList(vals);
}

/*
CKKS Bootstrapping functions
*/
void CKKSCryptoContext::evalBootstrapSetup() {
  std::vector<uint32_t> bsgsDim = {0, 0};
  std::vector<uint32_t> levelBudget = {4, 4};

  usint slots = context->GetEncodingParams()->GetBatchSize();
  context->EvalBootstrapSetup(levelBudget, bsgsDim, slots);
}

void CKKSCryptoContext::evalBootstrapKeyGen(
    const PrivateKey<DCRTPoly> &privateKey) {
  usint slots = context->GetEncodingParams()->GetBatchSize();
  // usint n = self.GetRingDimension();

  context->EvalBootstrapKeyGen(privateKey, slots);
}

boost::python::list CKKSCryptoContext::evalBootstrapList(boost::python::list ctxts) {

  std::vector<pyOpenFHE_CKKS::CKKSCiphertext> input_ctxts(len(ctxts));
  auto output_ctxts = pyOpenFHE::make_list(len(ctxts));

#pragma omp parallel for
  for (int i = 0; i < len(ctxts); ++i) {
    input_ctxts[i] = extract<pyOpenFHE_CKKS::CKKSCiphertext>(ctxts[i]);
    input_ctxts[i].cipher = context->EvalBootstrap(input_ctxts[i].cipher);
    output_ctxts[i] = input_ctxts[i];
  }

  return output_ctxts;
}

pyOpenFHE_CKKS::CKKSCiphertext CKKSCryptoContext::evalMetaBootstrap(pyOpenFHE_CKKS::CKKSCiphertext ctxt) {
    double error_scale = 1e-3;
    auto c2 = pyOpenFHE_CKKS::CKKSCiphertext(context->EvalBootstrap(ctxt.cipher));
    auto e1 = (ctxt - c2) * (1/error_scale);
    auto e2 = pyOpenFHE_CKKS::CKKSCiphertext(context->EvalBootstrap(e1.cipher)) * error_scale;
    auto c3 = c2 + e2;
    return c3;
}

boost::python::list CKKSCryptoContext::evalMetaBootstrapList(boost::python::list ctxts) {
    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> input_ctxts(len(ctxts));
    auto output_ctxts = pyOpenFHE::make_list(len(ctxts));

    for(int i = 0; i < len(ctxts); ++i) {
        input_ctxts[i] = extract<pyOpenFHE_CKKS::CKKSCiphertext>(ctxts[i]);
    }

    #pragma omp parallel for
    for(int i = 0; i < len(ctxts); ++i) {
        input_ctxts[i] = evalMetaBootstrap(input_ctxts[i]);
    }

    for(int i = 0; i < len(ctxts); ++i) {
        output_ctxts[i] = input_ctxts[i];
    }
    
    return output_ctxts;
}

pyOpenFHE_CKKS::CKKSCiphertext
CKKSCryptoContext::evalBootstrap(pyOpenFHE_CKKS::CKKSCiphertext ctxt) {
  ctxt.cipher = context->EvalBootstrap(ctxt.cipher);
  return ctxt;
}

// make rotation keys for all of the +/- powers-of-2
// we should probably try and put all the scheme-agnostic functions somewhere
// neutral reduce code duplication and C++ won't complain about it if we ever
// link these libraries
// TODO: that ^
void CKKSCryptoContext::evalPowerOf2RotationKeyGen(
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

} // namespace pyOpenFHE_CKKS