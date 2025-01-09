// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#include "ckks/CKKS_ciphertext_extension.hpp"
#include "ckks/cnn/poly.hpp"

#include <stdexcept>
#include <fmt/format.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/scope.hpp>
#include <omp.h>
#include <cstdlib>

#include "math/chebyshev.h"


double normalCDF(double value) {
   return 0.5 * erfc(-value * M_SQRT1_2);
}

double cpp_gelu(double x) {
    return x * normalCDF(x);
}

double cpp_gelu_scaled(double x, double bound=1.0) {
    return cpp_gelu(x * bound);
}

double cpp_relu(double x) {
    if(x < 0) return 0;
    return x;
}

boost::python::list pyOpenFHE_CKKS::fhe_gelu(const boost::python::list &py_shards, int degree, double bound) {

    int num_input_shards = len(py_shards);
    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> shards(num_input_shards);
    for(int i = 0 ; i < num_input_shards; ++i) {
        shards[i] = extract<pyOpenFHE_CKKS::CKKSCiphertext>(py_shards[i]);
    }

    int level = shards[0].getTowersRemaining() - 2;
    if(
        (level <= 2) || 
        ((degree <= 5) && (level < 3)) || 
        ((degree <= 13) && (degree >= 6) && (level < 4)) || 
        ((degree <= 27) && (degree >= 14) && (level < 5)) || 
        ((degree <= 59) && (degree >= 28) && (level < 6)) ||
        ((degree <= 119) && (degree >= 60) && (level < 7)) ||
        ((degree <= 200) && (degree >= 120) && (level < 8))
        ) {
        throw std::runtime_error(fmt::format("Insufficient number of towers remaining = {} to evaluate this Chebyshev series of degree = {}", level + 2, degree));
    }

    std::vector<double> coefficients = EvalChebyshevCoefficients([bound](double x) -> double { return cpp_gelu_scaled(x, bound); }, -1, 1, degree);

    #pragma omp parallel for
    for(int i = 0 ; i < num_input_shards; ++i) {
        auto cc = shards[i].cipher->GetCryptoContext();
        shards[i].cipher = cc->EvalChebyshevSeries(shards[i].cipher, coefficients, -1.0, 1.0);
    }

    boost::python::list res = pyOpenFHE::make_list(num_input_shards);
    for(int i = 0 ; i < num_input_shards; ++i) {
        res[i] = shards[i];
    }

    return res;

}