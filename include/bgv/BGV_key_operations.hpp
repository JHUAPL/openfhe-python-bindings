// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

// encrypt, decrypt, keygeneration, and the like

#ifndef BGV_ENCRYPTION_OPENFHE_PYTHON_BINDINGS_H
#define BGV_ENCRYPTION_OPENFHE_PYTHON_BINDINGS_H

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "bgv/BGV_ciphertext_extension.hpp"

#include "utils/utils.hpp"

using namespace boost::python;
using namespace boost::python::numpy;
using namespace lbcrypto;

namespace pyOpenFHE_BGV {
class BGVCiphertext;

// BGV-specific crypto context wrapper
class BGVCryptoContext {

public:
  // reminder to self that CryptoContext = shared_ptr<CryptoContextImpl>
  CryptoContext<DCRTPoly> context;

  // default for pickling
  BGVCryptoContext() {};

  BGVCryptoContext(CryptoContext<DCRTPoly> cc) : context(cc){};

  BGVCryptoContext(const BGVCryptoContext &cc) { context = cc.context; };

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
  pyOpenFHE_BGV::BGVCiphertext evalBootstrap(pyOpenFHE_BGV::BGVCiphertext);

  Plaintext encode(std::vector<int64_t>);

  pyOpenFHE_BGV::BGVCiphertext encryptPrivate(const PrivateKey<DCRTPoly> &,
                                              const list &);
  pyOpenFHE_BGV::BGVCiphertext encryptPrivate2(const PrivateKey<DCRTPoly> &,
                                               const ndarray &);

  pyOpenFHE_BGV::BGVCiphertext encryptPublic(const PublicKey<DCRTPoly> &,
                                             const list &);
  pyOpenFHE_BGV::BGVCiphertext encryptPublic2(const PublicKey<DCRTPoly> &,
                                              const ndarray &);

  ndarray decrypt(const PrivateKey<DCRTPoly> &, pyOpenFHE_BGV::BGVCiphertext &);

  usint getBatchSize() { return context->GetEncodingParams()->GetBatchSize(); };

  usint getRingDimension() { return context->GetRingDimension(); };

  usint getPlaintextModulus() {
    return context->GetEncodingParams()->GetPlaintextModulus();
  };

  ndarray zeroPadToBatchSize(std::vector<int64_t>);
  ndarray zeroPadToBatchSizeList(const list &);
  ndarray zeroPadToBatchSizeNumpy(const ndarray &);

  template <class Archive> void serialize(Archive &ar) { ar(context); };
};

// not entirely sure what the BGV crypto context is like
// TODO: verify this is the same
BGVCryptoContext genBGVContext(usint multiplicativeDepth, usint batchSize,
                               usint plaintextModulus,
                               SecurityLevel stdLevel = HEStd_128_classic,
                               usint ringDim = 0);

} // namespace pyOpenFHE_BGV

#endif /* BGV_ENCRYPTION_OPENFHE_PYTHON_BINDINGS_H */