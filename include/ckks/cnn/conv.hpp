// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef HE_CNN_CONV_H
#define HE_CNN_CONV_H

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

    boost::python::list conv2d(const boost::python::list &py_shards, const ndarray &npfilters, int mtx_size, const ndarray &permutation);

}

#endif
