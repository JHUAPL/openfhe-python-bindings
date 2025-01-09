// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#include "ckks/CKKS_ciphertext_extension.hpp"
#include "ckks/cnn/linear.hpp"

#include <stdexcept>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/scope.hpp>
#include <omp.h>
#include <cstdlib>

pyOpenFHE_CKKS::CKKSCiphertext pyOpenFHE_CKKS::linear(const boost::python::list &py_shards, const ndarray &npweights, const int mtx_size, const ndarray &permutation, const int pool_factor) {
    auto sigma = numpyListToCppLongIntVector(permutation);
    auto weights = numpyArrayToCppArray2D(npweights);

    int num_shards = len(py_shards);
    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> shards(num_shards);

    // #pragma omp parallel for
    for(int i = 0 ; i < num_shards; ++i) {
        shards[i] = extract<pyOpenFHE_CKKS::CKKSCiphertext>(py_shards[i]);
    }

    // do some math
    auto first_shard = shards[0];

    int num_outputs = weights.shape()[0];
    int num_inputs  = weights.shape()[1];
    int shard_size = first_shard.getBatchSize();
    int channel_size = mtx_size * mtx_size;
    int num_physical_channels_per_shard = shard_size / channel_size;

    int duplication_factor = 1;
    if (num_shards == 1) {
        duplication_factor = shard_size / num_inputs;
    }

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> partial_output(num_outputs * num_shards);

    #pragma omp parallel for collapse(2)
    for (int r = 0; r < num_outputs; ++r) {
        for (int s = 0; s < num_shards; s++) {
            std::vector<double> v(shard_size, 0.0);
            for (int i = 0; i < shard_size; ++i) {
                int physical_channel_idx = i / channel_size + s * num_physical_channels_per_shard;
                int logical_channel_idx = sigma[physical_channel_idx / duplication_factor];
                int channel_offset = i % channel_size;
                int idx = logical_channel_idx * channel_size + channel_offset;

                v[i] = weights[r][idx];
            }
            auto res = shards[s] * v;

            /*
            power-of-two add and rotate algorithm
            Note: we should be able to exit early if there is duplication,
            rather than going all the way down
            and then dividing by the duplication_factor afterwards.
            */
            int shift = shard_size / 2;
            while (shift > 0) {
                res = res + (res >> shift);
                shift /= 2;
            }

            // only keep a single output value
            std::vector<double> activ_mask(shard_size, 0.0);
            activ_mask[r] = 1.0 / (duplication_factor * pool_factor * pool_factor);
            res *= activ_mask;

            partial_output[r * num_shards + s] = res;
        }
    }

    auto enc_sum = partial_output[0];
    for(int r = 1; r < num_outputs * num_shards; ++r) {
        enc_sum += partial_output[r];
    }

    return enc_sum;
}