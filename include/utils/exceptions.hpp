// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef OpenFHE_PYTHON_EXCEPTIONS_H
#define OpenFHE_PYTHON_EXCEPTIONS_H

#include <stdexcept>

#include <boost/python.hpp>

using namespace boost::python;

namespace pyOpenFHE {

// we usually just throw RuntimeExceptions
// but certain exceptions are meaningful
// e.g. numpy will respond to NotImplementedError differently than TypeError
// so we need to define a custom exception on the C++ side and define a simple
// translator

// we inherit from logic_error because it has a string constructor, unlike
// exception
class not_implemented_exception : public std::logic_error {

  // constructor inheritance, thanks C++11
  using std::logic_error::logic_error;
};

class type_exception : public std::logic_error {
  using std::logic_error::logic_error;
};

void translate_not_implemented(not_implemented_exception const &e);
void translate_type_exception(type_exception const &e);

} // namespace pyOpenFHE

#endif /* OpenFHE_PYTHON_EXCEPTIONS_H */