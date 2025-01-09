// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

// bindings for the CryptoContext class

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "openfhe.h"

#include "bgv/BGV_key_operations.hpp"

using namespace boost::python;
using namespace boost::python::numpy;
using namespace lbcrypto;
using namespace pyOpenFHE;

namespace boost {
template <typename T> T *get_pointer(std::shared_ptr<T> p) { return p.get(); }
} // namespace boost

namespace pyOpenFHE_BGV {

// boost has trouble with overloaded methods
// best option is to define these function pointers for different method
// signatures then pass them later when def'ing the module other option: use
// lambda function pointers that basically do the same thing but in-line

// Minimum number of arguments is 4, maximum is 6 for genBGVContext
BOOST_PYTHON_FUNCTION_OVERLOADS(BGV_factory_overloads, genBGVContext, 3, 5)

void export_BGV_CryptoContext_boost() {

  class_<pyOpenFHE_BGV::BGVCryptoContext>("BGVCryptoContext",
                                          init<CryptoContext<DCRTPoly>>())
      .def("enable", &BGVCryptoContext::enable)
      .def("keyGen", &BGVCryptoContext::keyGen)
      .def("evalMultKeyGen", &BGVCryptoContext::evalMultKeyGen)
      .def("evalMultKeysGen", &BGVCryptoContext::evalMultKeysGen)
      .def("keySwitchGen", &BGVCryptoContext::keySwitchGen)
      .def("getSchemeID", &BGVCryptoContext::getSchemeId)

      /*
      boost doesn't accept lambdas as member functions
      but it *does* accept function pointers
      and the "+" turns this lambda into a function pointer
      this is only possible because it doesn't capture any values
      */
      .def("evalAtIndexKeyGen", &BGVCryptoContext::evalAtIndexKeyGen1)
      .def("evalAtIndexKeyGen", &BGVCryptoContext::evalAtIndexKeyGen2)
      .def("evalPowerOf2RotationKeyGen",
           &BGVCryptoContext::evalPowerOf2RotationKeyGen)
      .def("evalBootstrapSetup", &BGVCryptoContext::evalBootstrapSetup)
      .def("evalBootstrapKeyGen", &BGVCryptoContext::evalBootstrapKeyGen)
      .def("evalBootstrap", &BGVCryptoContext::evalBootstrap)
      .def("evalBootstrap", &BGVCryptoContext::evalBootstrapList)
      .def("encrypt", &BGVCryptoContext::encryptPublic)
      .def("encrypt", &BGVCryptoContext::encryptPrivate)
      .def("encrypt", &BGVCryptoContext::encryptPublic2)
      .def("encrypt", &BGVCryptoContext::encryptPrivate2)
      .def("decrypt", &BGVCryptoContext::decrypt)
      .def("getRingDimension", &BGVCryptoContext::getRingDimension)
      .def("getBatchSize", &BGVCryptoContext::getBatchSize)
      .def("getPlaintextModulus", &BGVCryptoContext::getPlaintextModulus)

      .def("zeroPadToBatchSize", &BGVCryptoContext::zeroPadToBatchSizeList)
      .def("zeroPadToBatchSize", &BGVCryptoContext::zeroPadToBatchSizeNumpy);

  def("genCryptoContextBGV", &genBGVContext,
      BGV_factory_overloads((arg("multiplicativeDepth"), arg("batchSize"),
                             arg("plaintextModulus"),
                             arg("stdLevel") = SecurityLevel::HEStd_128_classic,
                             arg("ringDim") = 0)));
}

} // namespace pyOpenFHE_BGV
