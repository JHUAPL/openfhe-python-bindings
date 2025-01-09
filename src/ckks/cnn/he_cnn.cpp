// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#include "ckks/CKKS_ciphertext_extension.hpp"
#include "ckks/bindings.hpp"
#include "ckks/cnn/he_cnn.hpp"
#include "ckks/cnn/pool.hpp"
#include "ckks/cnn/upsample.hpp"
#include "ckks/cnn/conv.hpp"
#include "ckks/cnn/linear.hpp"
#include "ckks/cnn/poly.hpp"
#include <boost/python/scope.hpp>
#include <omp.h>

using namespace pyOpenFHE;
using namespace pyOpenFHE_CKKS;
using namespace boost::python;
using namespace boost::python::numpy;

class boost_CNN {};

void pyOpenFHE_CKKS::export_he_cnn_functions_boost() {
    def("conv2d", conv2d);
    def("linear", linear);
    def("pool", pool);
    def("upsample", upsample);
    def("fhe_gelu", fhe_gelu);
    def("omp_set_num_threads", omp_set_num_threads);
    def("omp_set_nested", omp_set_nested);
    def("omp_set_dynamic", omp_set_dynamic);
}
