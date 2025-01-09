// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

/*
define all the serialization functions which operate on our wrapped ciphertext, crypto context, and keys here
*/

#include <stdexcept>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/tuple.hpp>

#include "ckks/CKKS_ciphertext_extension.hpp"
#include "ckks/CKKS_key_operations.hpp"
#include "ckks/CKKS_pickle.hpp"
#include "ckks/serialization.hpp"
#include "utils/utils.hpp"
#include "utils/enums_binding.hpp"

// header files needed for serialization
#include "openfhe.h"
#include "cryptocontext-ser.h"
#include "ciphertext-ser.h"
#include "key/key-ser.h"
#include "scheme/ckksrns/ckksrns-ser.h"

using namespace lbcrypto;

namespace pyOpenFHE_CKKS {

boost::python::tuple CKKSCiphertext_pickle_suite::getinitargs(const pyOpenFHE_CKKS::CKKSCiphertext& w) {
    return boost::python::make_tuple();
}

boost::python::tuple CKKSCiphertext_pickle_suite::getstate(const pyOpenFHE_CKKS::CKKSCiphertext& w) {
    PyObject * py_buffer = SerializeToBytes_Ciphertext(w, pyOpenFHE_CKKS::SerType::JSON);
    boost::python::handle<> handle(py_buffer);
    auto object = boost::python::object(handle);
    return boost::python::make_tuple(object);
}

void CKKSCiphertext_pickle_suite::setstate(pyOpenFHE_CKKS::CKKSCiphertext& w, boost::python::tuple state) {
    using namespace boost::python;
    if (len(state) != 1) {
        PyErr_SetObject(
        PyExc_ValueError,
        ("expected 1-item tuple in call to __setstate__; got %s" % state).ptr()
        );
        throw_error_already_set();
    }

    auto ctxt = DeserializeFromBytes_Ciphertext(state[0], pyOpenFHE_CKKS::SerType::JSON);
    w.cipher = ctxt.cipher;
}

}