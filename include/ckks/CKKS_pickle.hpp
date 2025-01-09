// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef CKKS_PICKLE_OPENFHE_PYTHON_BINDINGS_H
#define CKKS_PICKLE_OPENFHE_PYTHON_BINDINGS_H

#include <vector>
#include <complex>
#include <stdexcept>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "openfhe.h"
#include "utils/utils.hpp"

#include "ckks/CKKS_key_operations.hpp"
#include "ckks/CKKS_ciphertext_extension.hpp"
#include "ckks/serialization.hpp"


using namespace lbcrypto;

namespace pyOpenFHE_CKKS {

struct CKKSCiphertext_pickle_suite : boost::python::pickle_suite 
    {
        static boost::python::tuple getinitargs(const pyOpenFHE_CKKS::CKKSCiphertext& w);
        static boost::python::tuple getstate(const pyOpenFHE_CKKS::CKKSCiphertext& w);
        static void setstate(pyOpenFHE_CKKS::CKKSCiphertext& w, boost::python::tuple state);
    };

}

#endif /* CKKS_PICKLE_OPENFHE_PYTHON_BINDINGS_H */