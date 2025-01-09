// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#include <stdexcept>

#include <boost/python.hpp>

#include "utils/exceptions.hpp"

using namespace boost::python;

namespace pyOpenFHE {

void translate_not_implemented(not_implemented_exception const &e) {
  PyErr_SetString(PyExc_NotImplementedError, e.what());
}

void translate_type_exception(type_exception const &e) {
  PyErr_SetString(PyExc_TypeError, e.what());
}

} // namespace pyOpenFHE