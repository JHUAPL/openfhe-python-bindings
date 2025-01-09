// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef HE_CNN_PYTHON_BINDINGS_H
#define HE_CNN_PYTHON_BINDINGS_H

// functions for the homomorphic CNN

#include <vector>
#include <complex>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "boost/multi_array.hpp"
#include "utils/utils.hpp"

using namespace pyOpenFHE;
using namespace pyOpenFHE_CKKS;
using namespace boost::python;
using namespace boost::python::numpy;

namespace pyOpenFHE_CKKS {

    // should probably put this inside the OpenFHE namespace
    typedef typename boost::multi_array<pyOpenFHE_CKKS::CKKSCiphertext, 2> ciphertext_array2d;
    typedef typename boost::multi_array<pyOpenFHE_CKKS::CKKSCiphertext, 4> ciphertext_array4d;
}


#endif
