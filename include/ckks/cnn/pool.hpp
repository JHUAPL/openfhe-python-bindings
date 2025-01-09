// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef HE_CNN_POOL_H
#define HE_CNN_POOL_H

#include <vector>
#include <complex>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "boost/multi_array.hpp"
#include "utils/utils.hpp"
#include "ckks/CKKS_ciphertext_extension.hpp"

using namespace pyOpenFHE;
using namespace boost::python;
using namespace boost::python::numpy;

namespace pyOpenFHE_CKKS {

    boost::python::list pool(const boost::python::list &py_shards, int mtx_size, bool conv);
}

#endif
