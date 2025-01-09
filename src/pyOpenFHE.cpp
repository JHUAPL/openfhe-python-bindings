// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

// python bindings for OpenFHE's BGV and CKKS functionality

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
// nested scopes
#include <boost/python/scope.hpp>

#include "bgv/bindings.hpp"
#include "ckks/bindings.hpp"
#include "utils/enums_binding.hpp"
#include "utils/exceptions.hpp"

using namespace boost::python;
using namespace boost::python::numpy;

/*
boost doesn't natively support std::shared_ptr
anyway, the fix is easy, this will teach it to extract values from
shared pointers
*/
namespace boost {
template <typename T> T *get_pointer(std::shared_ptr<T> p) { return p.get(); }
} // namespace boost

// where the actual module is defined
BOOST_PYTHON_MODULE(pyOpenFHE) {
  Py_Initialize();

  // necessary for numpy to work
  boost::python::numpy::initialize();

  // register exception handlers
  register_exception_translator<pyOpenFHE::not_implemented_exception>(
      &pyOpenFHE::translate_not_implemented);
  register_exception_translator<pyOpenFHE::type_exception>(
      &pyOpenFHE::translate_type_exception);
  // get outermost scope, not sure if this is necessary
  scope package = scope();

     {
          using namespace pyOpenFHE;

          object enums_module(handle<>(borrowed(PyImport_AddModule("pyOpenFHE.enums"))));
          package.attr("enums") = enums_module;

          scope enums_scope = enums_module;
          
          export_enums_boost();
     }

     {
          using namespace pyOpenFHE_CKKS;

          object CKKS_module(handle<>(borrowed(PyImport_AddModule("pyOpenFHE.CKKS"))));
          package.attr("CKKS") = CKKS_module;

          scope CKKS_scope = CKKS_module;

          export_CKKS_CryptoContext_boost();
          export_CKKS_Ciphertext_boost();

          {
               object CKKS_serialization_module(handle<>(borrowed(PyImport_AddModule("pyOpenFHE.CKKS.serial"))));
               CKKS_scope.attr("serial") = CKKS_serialization_module;

               scope CKKS_serialization_scope = CKKS_serialization_module;

               export_CKKS_serialization_boost();
          }

          {
               object CKKS_CNN_module(handle<>(borrowed(PyImport_AddModule("pyOpenFHE.CKKS.CNN"))));
               CKKS_scope.attr("CNN") = CKKS_CNN_module;

               scope CKKS_CNN_scope = CKKS_CNN_module;

               export_he_cnn_functions_boost();
          }
     }

     {
          using namespace pyOpenFHE_BGV;

          // this supposedly makes "from pyOpenFHE.BGV import <thing>" work
          object BGV_module(handle<>(borrowed(PyImport_AddModule("pyOpenFHE.BGV"))));

          package.attr("BGV") = BGV_module;

          // so we no longer need a dummy class
          scope BGV_scope = BGV_module;

          export_BGV_CryptoContext_boost();
          export_BGV_Ciphertext_boost();
          {
               object BGV_serialization_module(handle<>(borrowed(PyImport_AddModule("pyOpenFHE.BGV.serial"))));
               BGV_scope.attr("serial") = BGV_serialization_module;

               scope BGV_serialization_scope = BGV_serialization_module;

               export_BGV_serialization_boost();
          }
     }

}