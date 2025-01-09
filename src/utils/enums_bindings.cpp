// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

// python bindings for OpenFHE's pke functionality

#include <stdexcept>

#include <fmt/format.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "bgv/BGV_key_operations.hpp"
#include "ckks/CKKS_key_operations.hpp"
#include "openfhe.h"

using namespace boost::python;
using namespace boost::python::numpy;
using namespace lbcrypto;

namespace pyOpenFHE {

void export_enums_boost() {

  // individual key classes
  // every "large" type is usually just typedef'd to be a shared pointer to an
  // "Impl" class so to make this seamless for python bindings, we wrap the
  // "Impl" class, but let boost know that sometimes (see: always) we contain
  // these objects in std::shared_ptr
  class_<PublicKeyImpl<DCRTPoly>, std::shared_ptr<PublicKeyImpl<DCRTPoly>>>(
      "PublicKey")
      .def("getCryptoContext",
           +[](PublicKeyImpl<DCRTPoly> &self) -> PyObject * {
             CryptoContext<DCRTPoly> cc = self.GetCryptoContext();
             if (cc->getSchemeId() == SCHEME::BGVRNS_SCHEME) {
               auto bgv_cc = pyOpenFHE_BGV::BGVCryptoContext(cc);
               return to_python_value<decltype(bgv_cc)>()(bgv_cc);
             } else if (cc->getSchemeId() == SCHEME::CKKSRNS_SCHEME) {
               auto ckks_cc = pyOpenFHE_CKKS::CKKSCryptoContext(cc);
               return to_python_value<decltype(ckks_cc)>()(ckks_cc);
             }

             throw std::runtime_error(
                 fmt::format("Unsupported encryption scheme: {}",
                             cc->GetScheme()->SerializedObjectName()));
           });

  class_<PrivateKeyImpl<DCRTPoly>, std::shared_ptr<PrivateKeyImpl<DCRTPoly>>>(
      "PrivateKey")
      .def("getCryptoContext",
           +[](PrivateKeyImpl<DCRTPoly> &self) -> PyObject * {
             CryptoContext<DCRTPoly> cc = self.GetCryptoContext();
             if (cc->getSchemeId() == SCHEME::BGVRNS_SCHEME) {
               auto bgv_cc = pyOpenFHE_BGV::BGVCryptoContext(cc);
               return to_python_value<decltype(bgv_cc)>()(bgv_cc);
             } else if (cc->getSchemeId() == SCHEME::CKKSRNS_SCHEME) {
               auto ckks_cc = pyOpenFHE_CKKS::CKKSCryptoContext(cc);
               return to_python_value<decltype(ckks_cc)>()(ckks_cc);
             }

             throw std::runtime_error(
                 fmt::format("Unsupported encryption scheme: {}",
                             cc->GetScheme()->SerializedObjectName()));
           });

  // combined public/private key. weird that the class is "PrivateKey" but the
  // data member is "secretKey", huh? anyway, the individual keys really
  // shouldn't be changed, I don't think it would break anything but why would
  // you do that "good" method makes sure the keys are valid
  class_<KeyPair<DCRTPoly>>("KeyPair",
                            init<PublicKey<DCRTPoly>, PrivateKey<DCRTPoly>>())
      .def_readonly("publicKey", &KeyPair<DCRTPoly>::publicKey)
      .def_readonly("secretKey", &KeyPair<DCRTPoly>::secretKey)
      .def("good", &KeyPair<DCRTPoly>::good);

  // not sure if this is necessary
  class_<EvalKeyImpl<DCRTPoly>, std::shared_ptr<EvalKeyImpl<DCRTPoly>>>(
      "EvalKey");

  // enums used with CryptoContext::Enable to enable certain features
  enum_<PKESchemeFeature>("PKESchemeFeature")
      .value("PKE", PKESchemeFeature::PKE)
      .value("KEYSWITCH", PKESchemeFeature::KEYSWITCH)
      .value("PRE", PKESchemeFeature::PRE)
      .value("LEVELEDSHE", PKESchemeFeature::LEVELEDSHE)
      .value("ADVANCEDSHE", PKESchemeFeature::ADVANCEDSHE)
      .value("MULTIPARTY", PKESchemeFeature::MULTIPARTY)
      .value("FHE", PKESchemeFeature::FHE);

  // enums used for constructing crypto contexts
  enum_<SecretKeyDist>("SecretKeyDist")
      .value("GAUSSIAN", SecretKeyDist::GAUSSIAN)
      .value("UNIFORM_TERNARY", SecretKeyDist::UNIFORM_TERNARY)
      .value("SPARSE_TERNARY", SecretKeyDist::SPARSE_TERNARY);

  enum_<ScalingTechnique>("ScalingTechnique")
      .value("FIXEDMANUAL", ScalingTechnique::FIXEDMANUAL)
      .value("FIXEDAUTO", ScalingTechnique::FIXEDAUTO)
      .value("FLEXIBLEAUTO", ScalingTechnique::FLEXIBLEAUTO)
      .value("FLEXIBLEAUTOEXT", ScalingTechnique::FLEXIBLEAUTOEXT)
      .value("NORESCALE", ScalingTechnique::NORESCALE)
      .value("INVALID_RS_TECHNIQUE", ScalingTechnique::INVALID_RS_TECHNIQUE);

  enum_<SecurityLevel>("SecurityLevel")
      .value("HEStd_128_classic", SecurityLevel::HEStd_128_classic)
      .value("HEStd_192_classic", SecurityLevel::HEStd_192_classic)
      .value("HEStd_256_classic", SecurityLevel::HEStd_256_classic)
      .value("HEStd_NotSet", SecurityLevel::HEStd_NotSet);

  enum_<EncryptionTechnique>("EncryptionTechnique")
      .value("STANDARD", EncryptionTechnique::STANDARD)
      .value("EXTENDED", EncryptionTechnique::EXTENDED);

  enum_<KeySwitchTechnique>("KeySwitchTechnique")
      .value("INVALID_KS_TECH", KeySwitchTechnique::INVALID_KS_TECH)
      .value("BV", KeySwitchTechnique::BV)
      .value("HYBRID", KeySwitchTechnique::HYBRID);

  enum_<MultiplicationTechnique>("MultiplicationTechnique")
      .value("BEHZ", MultiplicationTechnique::BEHZ)
      .value("HPS", MultiplicationTechnique::HPS)
      .value("HPSPOVERQ", MultiplicationTechnique::HPSPOVERQ)
      .value("HPSPOVERQLEVELED", MultiplicationTechnique::HPSPOVERQLEVELED);

  enum_<LargeScalingFactorConstants>("LargeScalingFactorConstants")
      .value("MAX_BITS_IN_WORD", LargeScalingFactorConstants::MAX_BITS_IN_WORD)
      .value("MAX_LOG_STEP", LargeScalingFactorConstants::MAX_LOG_STEP);

  // not sure if we want this (or need it), but getSchemeId is exported so I
  // figure this should be too
  enum_<SCHEME>("SCHEME")
      .value("INVALID_SCHEME", SCHEME::INVALID_SCHEME)
      .value("CKKSRNS_SCHEME", SCHEME::CKKSRNS_SCHEME)
      .value("BFVRNS_SCHEME", SCHEME::BFVRNS_SCHEME)
      .value("BGVRNS_SCHEME", SCHEME::BGVRNS_SCHEME);
}

} // namespace pyOpenFHE