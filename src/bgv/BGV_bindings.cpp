// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#include <complex>
#include <stdexcept>
#include <vector>

// string formatting for exceptions
#include <fmt/format.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "openfhe.h"

#include "bgv/BGV_ciphertext_extension.hpp"
#include "bgv/BGV_pickle.hpp"
#include "utils/utils.hpp"

using namespace boost::python;
using namespace boost::python::numpy;
using namespace lbcrypto;

namespace pyOpenFHE_BGV {

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(BGV_Rescale_overloads,
                                       pyOpenFHE_BGV::BGVCiphertext::rescale, 0,
                                       1)

void export_BGV_Ciphertext_boost() {

  class_<pyOpenFHE_BGV::BGVCiphertext>("BGVCiphertext",
                                       init<Ciphertext<DCRTPoly>>())
      .def(init<const pyOpenFHE_BGV::BGVCiphertext &>())
      // need to expose default constructor for pickling
      .def(init<>())
      .def("getPlaintextModulus",
           &pyOpenFHE_BGV::BGVCiphertext::getPlaintextModulus)
      .def("getScalingFactor", &pyOpenFHE_BGV::BGVCiphertext::getScalingFactor)
      .def("getBatchSize", &pyOpenFHE_BGV::BGVCiphertext::getBatchSize)
      .def("getMultLevel", &pyOpenFHE_BGV::BGVCiphertext::getMultLevel)
      .def("getTowersRemaining",
           &pyOpenFHE_BGV::BGVCiphertext::getTowersRemaining)
      .def("getCryptoContext", &pyOpenFHE_BGV::BGVCiphertext::getCryptoContext)
      .def("rescale", &pyOpenFHE_BGV::BGVCiphertext::rescale,
           BGV_Rescale_overloads((arg("levels") = 1)))
      .def("compress", &pyOpenFHE_BGV::BGVCiphertext::compress)

      .def("RotateEvalAtIndex", &pyOpenFHE_BGV::BGVRotateEvalAtIndex)
      .def("HoistedRotations", &pyOpenFHE_BGV::BGVHoistedRotations)

      .def("__copy__",
           +[](pyOpenFHE_BGV::BGVCiphertext &self) {
             return pyOpenFHE_BGV::BGVCiphertext(self);
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
      .def(self += int64_t())
      .def(self + int64_t())
      .def(int64_t() + self)
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
      .def(self -= int64_t())
      .def(self - int64_t())
      .def(int64_t() - self)
      .def(self *= int64_t())
      .def(self * int64_t())
      .def(int64_t() * self)
      .def(self *= other<list>())
      .def(self *= other<ndarray>())
      .def(self * other<list>())
      .def(self * other<ndarray>())
      .def(other<list>() * self)
      .def(other<ndarray>() * self)

      // should prevent weird numpy broadcasting
      .def("__array_ufunc__", &pyOpenFHE_BGV::BGVCiphertext::array_ufunc)

      // attempt to support pickling
      .def_pickle(BGVCiphertext_pickle_suite())
      .attr("__module__") = "pyOpenFHE.BGV";
}

} // namespace pyOpenFHE_BGV
