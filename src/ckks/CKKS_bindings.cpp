// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

// python bindings for OpenFHE's implementation of CKKS

#include <complex>
#include <stdexcept>
#include <vector>

#include <fmt/format.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "openfhe.h"

#include "ckks/CKKS_ciphertext_extension.hpp"
#include "ckks/CKKS_pickle.hpp"
#include "utils/utils.hpp"

using namespace boost::python;
using namespace boost::python::numpy;
using namespace lbcrypto;

namespace pyOpenFHE_CKKS {

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(CKKS_Rescale_overloads,
                                       pyOpenFHE_CKKS::CKKSCiphertext::Rescale,
                                       0, 1)

void export_CKKS_Ciphertext_boost() {

  class_<pyOpenFHE_CKKS::CKKSCiphertext>("CKKSCiphertext",
                                         init<Ciphertext<DCRTPoly>>())
      .def(init<const pyOpenFHE_CKKS::CKKSCiphertext &>())
      .def(init<>()) // for pickle
      .def("getPlaintextModulus",
           &pyOpenFHE_CKKS::CKKSCiphertext::getPlaintextModulus)
      .def("getScalingFactor",
           &pyOpenFHE_CKKS::CKKSCiphertext::getScalingFactor)
      .def("getBatchSize", &pyOpenFHE_CKKS::CKKSCiphertext::getBatchSize)
      .def("getMultLevel", &pyOpenFHE_CKKS::CKKSCiphertext::getMultLevel)
      .def("getTowersRemaining",
           &pyOpenFHE_CKKS::CKKSCiphertext::getTowersRemaining)
      .def("getCryptoContext",
           &pyOpenFHE_CKKS::CKKSCiphertext::getCryptoContext)
      .def("Rescale", &pyOpenFHE_CKKS::CKKSCiphertext::Rescale,
           CKKS_Rescale_overloads((arg("levels") = 1)))
      .def("compress", &pyOpenFHE_CKKS::CKKSCiphertext::compress)

      .def("RotateEvalAtIndex", &pyOpenFHE_CKKS::CKKSRotateEvalAtIndex)
      .def("HoistedRotations", &pyOpenFHE_CKKS::CKKSHoistedRotations)
      .def("MultiplySingletonDirect",
           &pyOpenFHE_CKKS::CKKSMultiplySingletonDirect)
      .def("MultiplySingletonIntDoubleAndAdd",
           &pyOpenFHE_CKKS::CKKSMultiplySingletonIntDoubleAndAdd)

      .def("__copy__",
           +[](pyOpenFHE_CKKS::CKKSCiphertext &self) {
             return pyOpenFHE_CKKS::CKKSCiphertext(self);
           })
      // here we specify every possible type interaction:
      .def(self == self)
      .def(self != self)
      .def(self += self)
      .def(self + self)
      .def(self -= self)
      .def(self - self)
      .def(self *= self)
      .def(self * self)
      .def(self <<= int())
      .def(self >>= int())
      .def(self << int())
      .def(self >> int())
      .def(self += double())
      .def(self + double())
      .def(double() + self)
      .def(self += other<list>())
      .def(self += other<ndarray>())
      .def(self + other<list>())
      .def(self + other<ndarray>())
      .def(other<list>() + self)
      .def(other<ndarray>() + self)
      .def(self -= other<list>())
      .def(self -= other<ndarray>())
      .def(self - other<list>())
      .def(self - other<ndarray>())
      .def(-self)
      .def(other<list>() - self)
      .def(other<ndarray>() - self)
      .def(self -= double())
      .def(self - double())
      .def(double() - self)
      .def(self *= double())
      .def(self * double())
      .def(double() * self)
      .def(self *= other<list>())
      .def(self *= other<ndarray>())
      .def(self * other<list>())
      .def(self * other<ndarray>())
      .def(other<list>() * self)
      .def(other<ndarray>() * self)
      .def("__array_ufunc__", &pyOpenFHE_CKKS::CKKSCiphertext::array_ufunc)
      .def_pickle(CKKSCiphertext_pickle_suite())
      .attr("__module__") = "pyOpenFHE.CKKS";
}

} // namespace pyOpenFHE_CKKS