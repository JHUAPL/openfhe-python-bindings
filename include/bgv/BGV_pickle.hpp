// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef BGV_PICKLE_OPENFHE_PYTHON_BINDINGS_H
#define BGV_PICKLE_OPENFHE_PYTHON_BINDINGS_H

#include <complex>
#include <stdexcept>
#include <vector>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "openfhe.h"
#include "utils/utils.hpp"

#include "bgv/BGV_ciphertext_extension.hpp"
#include "bgv/BGV_key_operations.hpp"
#include "bgv/serialization.hpp"

using namespace lbcrypto;

namespace pyOpenFHE_BGV {

struct BGVCiphertext_pickle_suite : boost::python::pickle_suite {
  static boost::python::tuple
  getinitargs(const pyOpenFHE_BGV::BGVCiphertext &w);
  static boost::python::tuple getstate(const pyOpenFHE_BGV::BGVCiphertext &w);
  static void setstate(pyOpenFHE_BGV::BGVCiphertext &w,
                       boost::python::tuple state);
};

} // namespace pyOpenFHE_BGV

#endif /* BGV_PICKLE_OPENFHE_PYTHON_BINDINGS_H */
