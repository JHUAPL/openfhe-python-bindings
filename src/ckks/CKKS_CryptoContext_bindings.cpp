// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

// bindings for the CryptoContext class

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "openfhe.h"

#include "ckks/CKKS_key_operations.hpp"

using namespace boost::python;
using namespace boost::python::numpy;
using namespace lbcrypto;

namespace boost {
template <typename T> T *get_pointer(std::shared_ptr<T> p) { return p.get(); }
} // namespace boost

namespace pyOpenFHE_CKKS {

// boost has trouble with overloaded methods
// best option is to define these function pointers for different method
// signatures then pass them later when def'ing the module other option: use
// lambda function pointers that basically do the same thing but in-line
void (CryptoContextImpl<DCRTPoly>::*Enable1)(PKESchemeFeature) =
    &CryptoContextImpl<DCRTPoly>::Enable;

// Minimum number of arguments is 3, maximum is 5 for genCKKSContext
BOOST_PYTHON_FUNCTION_OVERLOADS(CKKS_factory_overloads, genCKKSContext, 3, 5)

void export_CKKS_CryptoContext_boost() {

  class_<pyOpenFHE_CKKS::CKKSCryptoContext>("CKKSCryptoContext",
                                            init<CryptoContext<DCRTPoly>>())
      .def("enable", &CKKSCryptoContext::enable)
      .def("keyGen", &CKKSCryptoContext::keyGen)
      .def("evalMultKeyGen", &CKKSCryptoContext::evalMultKeyGen)
      .def("evalMultKeysGen", &CKKSCryptoContext::evalMultKeysGen)
      .def("keySwitchGen", &CKKSCryptoContext::keySwitchGen)
      .def("getSchemeID", &CKKSCryptoContext::getSchemeId)

      /*
      boost doesn't accept lambdas as member functions
      but it *does* accept function pointers
      and the "+" turns this lambda into a function pointer
      this is only possible because it doesn't capture any values
      */
      .def("evalAtIndexKeyGen", &CKKSCryptoContext::evalAtIndexKeyGen1)
      .def("evalAtIndexKeyGen", &CKKSCryptoContext::evalAtIndexKeyGen2)
      .def("evalPowerOf2RotationKeyGen",
           &CKKSCryptoContext::evalPowerOf2RotationKeyGen)
      .def("evalBootstrapSetup", &CKKSCryptoContext::evalBootstrapSetup)
      .def("evalBootstrapKeyGen", &CKKSCryptoContext::evalBootstrapKeyGen)
      .def("evalBootstrap", &CKKSCryptoContext::evalBootstrap)
      .def("evalBootstrap", &CKKSCryptoContext::evalBootstrapList)
      .def("evalMetaBootstrap", &CKKSCryptoContext::evalMetaBootstrap)
      .def("evalMetaBootstrap", &CKKSCryptoContext::evalMetaBootstrapList)
      .def("encrypt", &CKKSCryptoContext::encryptPublic)
      .def("encrypt", &CKKSCryptoContext::encryptPrivate)
      .def("encrypt", &CKKSCryptoContext::encryptPublic2)
      .def("encrypt", &CKKSCryptoContext::encryptPrivate2)
      .def("decrypt", &CKKSCryptoContext::decrypt)
      .def("getRingDimension", &CKKSCryptoContext::getRingDimension)
      .def("getBatchSize", &CKKSCryptoContext::getBatchSize)

      .def("zeroPadToBatchSize", &CKKSCryptoContext::zeroPadToBatchSizeList)
      .def("zeroPadToBatchSize", &CKKSCryptoContext::zeroPadToBatchSizeNumpy);

  def("genCryptoContextCKKS", &genCKKSContext,
      CKKS_factory_overloads(
          (arg("multiplicativeDepth"), arg("scalingFactorBits"),
           arg("batchSize"), arg("stdLevel") = SecurityLevel::HEStd_128_classic,
           arg("ringDim") = 0)));
}

} // namespace pyOpenFHE_CKKS