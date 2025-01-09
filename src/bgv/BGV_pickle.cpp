// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

/*
define all the serialization functions which operate on our wrapped ciphertext,
crypto context, and keys here
*/

#include <stdexcept>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/tuple.hpp>

#include "bgv/BGV_ciphertext_extension.hpp"
#include "bgv/BGV_key_operations.hpp"
#include "bgv/BGV_pickle.hpp"
#include "bgv/serialization.hpp"
#include "utils/enums_binding.hpp"
#include "utils/utils.hpp"

// header files needed for serialization
#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "key/key-ser.h"
#include "openfhe.h"
#include "scheme/bgvrns/bgvrns-ser.h"

using namespace lbcrypto;

namespace pyOpenFHE_BGV {

boost::python::tuple
BGVCiphertext_pickle_suite::getinitargs(const pyOpenFHE_BGV::BGVCiphertext &w) {
  return boost::python::make_tuple();
}

boost::python::tuple
BGVCiphertext_pickle_suite::getstate(const pyOpenFHE_BGV::BGVCiphertext &w) {
  PyObject *py_buffer =
      SerializeToBytes_Ciphertext(w, pyOpenFHE_BGV::SerType::JSON);
  boost::python::handle<> handle(py_buffer);
  auto object = boost::python::object(handle);
  return boost::python::make_tuple(object);
}

void BGVCiphertext_pickle_suite::setstate(pyOpenFHE_BGV::BGVCiphertext &w,
                                          boost::python::tuple state) {
  using namespace boost::python;
  if (len(state) != 1) {
    PyErr_SetObject(
        PyExc_ValueError,
        ("expected 1-item tuple in call to __setstate__; got %s" % state)
            .ptr());
    throw_error_already_set();
  }

  auto ctxt =
      DeserializeFromBytes_Ciphertext(state[0], pyOpenFHE_BGV::SerType::JSON);
  w.cipher = ctxt.cipher;
}

} // namespace pyOpenFHE_BGV