// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

// encrypt, decrypt, keygeneration, and the like

#ifndef CKKS_ENCRYPTION_OPENFHE_PYTHON_BINDINGS_H
#define CKKS_ENCRYPTION_OPENFHE_PYTHON_BINDINGS_H

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "ckks/CKKS_ciphertext_extension.hpp"
#include "utils/utils.hpp"

using namespace boost::python;
using namespace boost::python::numpy;
using namespace lbcrypto;

namespace pyOpenFHE_CKKS {
class CKKSCiphertext;

// CKKS-specific crypto context wrapper
class CKKSCryptoContext {

public:
  // reminder to self that CryptoContext = shared_ptr<CryptoContextImpl>
  CryptoContext<DCRTPoly> context;

  CKKSCryptoContext(CryptoContext<DCRTPoly> cc) : context(cc){};

  CKKSCryptoContext(const CKKSCryptoContext &cc) { context = cc.context; };

  void enable(PKESchemeFeature m) { context->Enable(m); };

  KeyPair<DCRTPoly> keyGen() { return context->KeyGen(); };

  void evalMultKeyGen(const PrivateKey<DCRTPoly> privateKey) {
    context->EvalMultKeyGen(privateKey);
  };
  void evalMultKeysGen(const PrivateKey<DCRTPoly> privateKey) {
    context->EvalMultKeysGen(privateKey);
  };

  EvalKey<DCRTPoly> keySwitchGen(const PrivateKey<DCRTPoly> oldPrivateKey,
                                 const PrivateKey<DCRTPoly> newPrivateKey) {
    return context->KeySwitchGen(oldPrivateKey, newPrivateKey);
  };

  SCHEME getSchemeId() { return context->getSchemeId(); }

  void evalAtIndexKeyGen1(const PrivateKey<DCRTPoly> privateKey,
                          const list &index_list) {
    context->EvalAtIndexKeyGen(
        privateKey, pyOpenFHE::pythonListToCppIntVector(index_list), nullptr);
  };

  void evalAtIndexKeyGen2(const PrivateKey<DCRTPoly> privateKey,
                          const ndarray &index_list) {
    context->EvalAtIndexKeyGen(
        privateKey, pyOpenFHE::numpyListToCppIntVector(index_list), nullptr);
  };

  void evalPowerOf2RotationKeyGen(const PrivateKey<DCRTPoly> &);

  void evalBootstrapSetup();
  void evalBootstrapKeyGen(const PrivateKey<DCRTPoly> &);
  list evalBootstrapList(list);
  pyOpenFHE_CKKS::CKKSCiphertext evalBootstrap(pyOpenFHE_CKKS::CKKSCiphertext);
  list evalMetaBootstrapList(list);
  pyOpenFHE_CKKS::CKKSCiphertext evalMetaBootstrap(pyOpenFHE_CKKS::CKKSCiphertext);

  Plaintext encode(std::vector<double>);

  pyOpenFHE_CKKS::CKKSCiphertext encryptPrivate(const PrivateKey<DCRTPoly> &,
                                                const list &);
  pyOpenFHE_CKKS::CKKSCiphertext encryptPrivate2(const PrivateKey<DCRTPoly> &,
                                                 const ndarray &);

  pyOpenFHE_CKKS::CKKSCiphertext encryptPublic(const PublicKey<DCRTPoly> &,
                                               const list &);
  pyOpenFHE_CKKS::CKKSCiphertext encryptPublic2(const PublicKey<DCRTPoly> &,
                                                const ndarray &);

  ndarray decrypt(const PrivateKey<DCRTPoly> &,
                  pyOpenFHE_CKKS::CKKSCiphertext &);

  size_t getBatchSize() {
    return context->GetEncodingParams()->GetBatchSize();
  };

  size_t getRingDimension() { return context->GetRingDimension(); };

  ndarray zeroPadToBatchSize(std::vector<double>);
  ndarray zeroPadToBatchSizeList(const list &);
  ndarray zeroPadToBatchSizeNumpy(const ndarray &);

  template <class Archive> void serialize(Archive &ar) { ar(context); };
};

CKKSCryptoContext genCKKSContext(usint multiplicativeDepth,
                                 usint scalingFactorBits, usint batchSize,
                                 SecurityLevel stdLevel = HEStd_128_classic,
                                 usint ringDim = 0);
} // namespace pyOpenFHE_CKKS

#endif