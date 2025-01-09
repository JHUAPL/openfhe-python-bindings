// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#include "ckks/CKKS_ciphertext_extension.hpp"
#include "ckks/cnn/he_cnn.hpp"
#include "ckks/cnn/conv.hpp"
#include "utils/utils.hpp"
#include "ckks/utils.hpp"

#include <stdexcept>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/scope.hpp>
#include <omp.h>
#include <cstdlib>

using namespace pyOpenFHE;
using namespace pyOpenFHE_CKKS;
using namespace boost::python;
using namespace boost::python::numpy;

pyOpenFHE_CKKS::ciphertext_array2d get_all_rotations_image_sharded(pyOpenFHE_CKKS::CKKSCiphertext &ciphertext, int mtx_size, int ker_size) {
    pyOpenFHE_CKKS::ciphertext_array2d rotations(boost::extents[ker_size][ker_size]);

    int center = shift_to_kernel_index(0, ker_size);
    
    // fill out one row
    for (int j = 0; j < ker_size; j++) {
        int shift = kernel_index_to_shift(j, ker_size);
        rotations[center][j] = ciphertext << shift;
    }

    // fill out the rest
    for (int i = center - 1; i >= 0; i--) {
        for (int j = 0; j < ker_size; j++) {
            rotations[i][j] = rotations[i+1][j] >> mtx_size;
        }
    }
    for (int i = center + 1; i < ker_size; i++) {
        for (int j = 0; j < ker_size; j++) {
            rotations[i][j] = rotations[i-1][j] << mtx_size;
        }
    }

    return rotations;
}

pyOpenFHE_CKKS::ciphertext_array4d get_all_rotations_channel_sharded(std::vector<pyOpenFHE_CKKS::CKKSCiphertext>& all_shards, int shards_per_channel, int mtx_size, int ker_size) {
    int num_channels = all_shards.size() / shards_per_channel;
    pyOpenFHE_CKKS::ciphertext_array4d rotations(boost::extents[num_channels][shards_per_channel][ker_size][ker_size]);

    for (int channel_index = 0; channel_index < num_channels; channel_index++) {
        int channel_offset = channel_index * shards_per_channel;
        for (int shard_index = 0; shard_index < shards_per_channel; shard_index++) {
            int center = shift_to_kernel_index(0, ker_size);
        
            // fill out one row
            for (int j = 0; j < ker_size; j++) {
                int shift = kernel_index_to_shift(j, ker_size);
                rotations[channel_index][shard_index][center][j] = all_shards[channel_offset + shard_index] << shift;
            }

            // fill out the rest
            for (int i = center - 1; i >= 0; i--) {
                for (int j = 0; j < ker_size; j++) {
                    rotations[channel_index][shard_index][i][j] = rotations[channel_index][shard_index][i+1][j] >> mtx_size;
                }
            }
            for (int i = center + 1; i < ker_size; i++) {
                for (int j = 0; j < ker_size; j++) {
                    rotations[channel_index][shard_index][i][j] = rotations[channel_index][shard_index][i-1][j] << mtx_size;
                }
            }
        }
    }
    
    return rotations;
}

// looks and dimensions of 4d filters multiarray to determine duplication factors
pyOpenFHE_CKKS::CKKSCiphertext convolution_helper_image_sharded(const pyOpenFHE_CKKS::ciphertext_array2d &ciphertext_rotations,
                                                boost_vector4d &filters,
                                                int mtx_size,
                                                int r,
                                                int num_in_channels_per_shard,
                                                int num_out_channels_per_shard,
                                                int fragment_offset,
                                                int shard_offset,
                                                std::vector<long int> &sigma) {
    // math!
    auto ciphertext = ciphertext_rotations[0][0];
    int shard_size = ciphertext.getBatchSize();
    int ker_size = filters.shape()[2]; // assuming square kernels
    int channel_size = mtx_size * mtx_size;
    int num_physical_channels = shard_size / channel_size;
    int input_dup_factor = shard_size / (num_in_channels_per_shard * channel_size);
    int output_dup_factor = shard_size / (num_out_channels_per_shard * channel_size);

    auto enc_sum = ciphertext - ciphertext; // zero

    std::vector<double> kernel_elements(num_physical_channels);
    std::vector<double> masked_kernel_elements(shard_size);

    // iterate over all rotations
    for (int ki = 0; ki < ker_size; ki++) {
        int num_shift_ud = kernel_index_to_shift(ki, ker_size);
        for (int kj = 0; kj < ker_size; kj++) {
            int num_shift_lr = kernel_index_to_shift(kj, ker_size);
            auto mask = make_shift_mask_image_sharded(num_physical_channels, mtx_size, mtx_size, num_shift_ud, num_shift_lr);

            // extract elements from filters...
            for (int idx = 0; idx < num_physical_channels; idx++) {
                int i = (num_in_channels_per_shard * fragment_offset) + (idx / input_dup_factor + r) % num_in_channels_per_shard;
                int sigma_i = (int)sigma[i];
                int adjustment = (input_dup_factor / output_dup_factor);
                if(adjustment == 0) adjustment = 1;
                int j = (num_out_channels_per_shard * shard_offset) + (idx / output_dup_factor);
                kernel_elements[idx] = filters[sigma_i][j][ki][kj];
            }

            // and mask them, possibly with repetition
            for (int i = 0; i < shard_size; i++) {
                int idx = ((i / channel_size) - (r * input_dup_factor) + num_physical_channels) % num_physical_channels;
                masked_kernel_elements[i] = mask[i] * kernel_elements[idx];
            }

            enc_sum += ciphertext_rotations[ki][kj] * masked_kernel_elements;
        }
    }

    enc_sum <<= (r * mtx_size * mtx_size * input_dup_factor);

    return enc_sum;
}

pyOpenFHE_CKKS::CKKSCiphertext convolution_helper_channel_sharded(pyOpenFHE_CKKS::ciphertext_array4d& rotations,
                                                                boost_vector4d &filters,
                                                                int mtx_size,
                                                                int channel_index,
                                                                int channel_shard_index,
                                                                int output_channel_index) {
    auto first_shard = rotations[0][0][0][0];
    int shard_size = first_shard.getBatchSize();
    int channel_size = mtx_size * mtx_size;
    int shards_per_channel = channel_size / shard_size;
    int ker_size = filters.shape()[2];

    int num_rows = mtx_size / shards_per_channel;
    int num_cols = mtx_size;

    auto enc_sum = first_shard - first_shard; // zero

    std::vector<double> masked_kernel_elements(shard_size);
    std::vector<double> bleed_masked_kernel_elements(shard_size);

    // iterate over all shifts
    for (int ki = 0; ki < ker_size; ki++) {
        int num_shift_ud = kernel_index_to_shift(ki, ker_size);

        // TODO move this computation into a separate function
        int bleed_shard_index;
        if (num_shift_ud > 0) {
            if (channel_shard_index + 1 == shards_per_channel) {
                bleed_shard_index = -1;
            } else {
                bleed_shard_index = channel_shard_index + 1;
            }
        } else if (num_shift_ud < 0) {
            if (channel_shard_index == 0) {
                bleed_shard_index = -1;
            } else {
                bleed_shard_index = channel_shard_index - 1;
            }
        } else {
            bleed_shard_index = -1;
        }

        for (int kj = 0; kj < ker_size; kj++) {
            int num_shift_lr = kernel_index_to_shift(kj, ker_size);

            auto mask = make_shift_mask_channel_shard(num_rows, num_cols, num_shift_ud, num_shift_lr);
            auto bleed_mask = make_shift_mask_bleed_channel_shard(num_rows, num_cols, num_shift_ud, num_shift_lr);
            auto kernel_element = filters[channel_index][output_channel_index][ki][kj];

            // create masked kernel elements
            for (int i = 0; i < shard_size; i++) {
                masked_kernel_elements[i] = mask[i] * kernel_element;
                bleed_masked_kernel_elements[i] = bleed_mask[i] * kernel_element;
            }

            enc_sum += rotations[channel_index][channel_shard_index][ki][kj] * masked_kernel_elements;
            if (bleed_shard_index >= 0) {
                enc_sum += rotations[channel_index][bleed_shard_index][ki][kj] * bleed_masked_kernel_elements;
            }
        }
    }

    return enc_sum;
}

//  or not we have channel shards or not (can pretty easily do this by mathing it out, as below).
boost::python::list conv2d_image_sharded(std::vector<pyOpenFHE_CKKS::CKKSCiphertext> & shards, const ndarray &npfilters, int mtx_size, const ndarray &permutation) {
    // convert to boost multiarray
    auto filters = numpyArrayToCppArray4D(npfilters);
    auto sigma = numpyListToCppLongIntVector(permutation);

    auto first_shard = shards[0];

    // do some math
    // filter dims: input channels, output channels, kernel size x, kernel size y
    int num_input_shards = shards.size();
    int shard_size = first_shard.getBatchSize();
    int channel_size = mtx_size * mtx_size; // assuming square matrices, may want to change this assumption later though
    int num_physical_channels_per_shard = shard_size / channel_size;
    int num_output_channels = filters.shape()[1];
    int ker_size = filters.shape()[2];
    int num_output_shards = num_output_channels / num_physical_channels_per_shard;

    // if this is true, output_dup_factor > 1
    if (num_output_shards == 0) {
        num_output_shards = 1;
    }

    // these are needed to slice filters correctly
    // TODO can probably simplify this logic
    int num_in_channels_per_shard;
    if (num_input_shards > 1) {
        // no duplication
        num_in_channels_per_shard = num_physical_channels_per_shard;
    } else {
        // number of channels in the single shard is the total number of channels,
        // can be determined by looking at dimension of filters
        num_in_channels_per_shard = filters.shape()[0];
    }
    int num_out_channels_per_shard;
    if (num_output_shards > 1) {
        num_out_channels_per_shard = num_physical_channels_per_shard;
    } else {
        num_out_channels_per_shard = filters.shape()[1];
    }

    // using convolution_helper, compute one partial output shard at a time
    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> partial_convolutions(num_in_channels_per_shard * num_output_shards * num_input_shards);

    boost::multi_array<pyOpenFHE_CKKS::CKKSCiphertext, 3> all_ciphertext_rotations(boost::extents[num_input_shards][ker_size][ker_size]);
    #pragma omp parallel for
    for (int f = 0; f < num_input_shards; f++) {
        all_ciphertext_rotations[f] = get_all_rotations_image_sharded(shards[f], mtx_size, ker_size);
    }
    
    #pragma omp parallel for collapse(3)
    for (int s = 0; s < num_output_shards; s++) {
        for (int f = 0; f < num_input_shards; f++) {
            for(int r = 0; r < num_in_channels_per_shard; ++r) {
                int idx = s * (num_in_channels_per_shard * num_input_shards) + f * num_in_channels_per_shard + r;
                partial_convolutions[idx] = convolution_helper_image_sharded(
                    all_ciphertext_rotations[f],
                    filters,
                    mtx_size,
                    r,
                    num_in_channels_per_shard,
                    num_out_channels_per_shard,
                    f,
                    s,
                    sigma
                );
            }
        }
    }

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> output_shards(num_output_shards);
    #pragma omp parallel for
    for (int s = 0; s < num_output_shards; s++) {
        int idx = s * (num_in_channels_per_shard * num_input_shards);
        auto ctxt = partial_convolutions[idx];
        for(int offset = 1; offset < num_in_channels_per_shard * num_input_shards; ++offset) {
            ctxt += partial_convolutions[idx + offset];
        }
        output_shards[s] = ctxt;
    }

    boost::python::list res = pyOpenFHE::make_list(num_output_shards);
    for(int s = 0 ; s < num_output_shards; ++s) {
        res[s] = output_shards[s];
    }

    return res;
}

// entry point for channel sharding
boost::python::list conv2d_channel_sharded(std::vector<pyOpenFHE_CKKS::CKKSCiphertext> &shards, const ndarray &npfilters, int mtx_size) {
    auto filters = numpyArrayToCppArray4D(npfilters);
    auto first_shard = shards[0];

    // math!
    int num_input_shards = shards.size();
    int shard_size = first_shard.getBatchSize();
    int channel_size = mtx_size * mtx_size;
    int shards_per_channel = channel_size / shard_size;
    int num_input_channels = num_input_shards / shards_per_channel;
    int num_output_channels = filters.shape()[1];
    int ker_size = filters.shape()[2];
    int num_output_shards = num_output_channels * shards_per_channel;

    // cache partial computations here
    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> partial_convolutions(num_input_channels * shards_per_channel * num_output_channels);

    auto channel_shard_rotations = get_all_rotations_channel_sharded(shards, shards_per_channel, mtx_size, ker_size);

    // quintuply nested loop!
    #pragma omp parallel for collapse(3)
    for (int input_channel_index = 0; input_channel_index < num_input_channels; input_channel_index++) {
        for (int channel_shard_index = 0; channel_shard_index < shards_per_channel; channel_shard_index++) {
            for (int output_channel_index = 0; output_channel_index < num_output_channels; output_channel_index++) {
                int idx = input_channel_index * (shards_per_channel * num_output_channels) + output_channel_index * shards_per_channel + channel_shard_index;

                // just pass in all shards since we'll need to reference adjacent ones
                partial_convolutions[idx] = convolution_helper_channel_sharded(
                    channel_shard_rotations,
                    filters,
                    mtx_size,
                    input_channel_index,
                    channel_shard_index,
                    output_channel_index
                );
            }
        }
    }

    // Sum up the partial_convolutions
    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> output_shards(num_output_shards);

    for (int output_channel_index = 0; output_channel_index < num_output_channels; output_channel_index++) {

        // Loop over shards
        for (int shard_index = 0; shard_index < shards_per_channel; shard_index++) {

            auto enc_sum = first_shard - first_shard; // zero

            // Loop over input channels
            for (int input_channel_index = 0; input_channel_index < num_input_channels; input_channel_index++) {

                // Calculate offset
                int idx = input_channel_index * (shards_per_channel * num_output_channels) + output_channel_index * shards_per_channel + shard_index;
                enc_sum += partial_convolutions[idx];
            }

            int output_idx = output_channel_index * shards_per_channel + shard_index;
            output_shards[output_idx] = enc_sum;
        }
    }

    boost::python::list res = pyOpenFHE::make_list(num_output_shards);
    for(int s = 0 ; s < num_output_shards; ++s) {
        res[s] = output_shards[s];
    }

    return res;
}

boost::python::list pyOpenFHE_CKKS::conv2d(const boost::python::list &py_shards, const ndarray &npfilters, int mtx_size, const ndarray &permutation) {
    // extract objects
    int num_input_shards = len(py_shards);
    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> shards(num_input_shards);
    for (int i = 0 ; i < num_input_shards; ++i) {
        shards[i] = extract<pyOpenFHE_CKKS::CKKSCiphertext>(py_shards[i]);
    }

    // based on channel size vs shard size, invoke corresponding function
    int shard_size = shards[0].getBatchSize();
    int channel_size = mtx_size * mtx_size;

    if (shard_size >= channel_size) {
        return conv2d_image_sharded(shards, npfilters, mtx_size, permutation);
    } else {
        // A conv on a channel-sharded image won't have permuted channels, so ignore the permutation
        return conv2d_channel_sharded(shards, npfilters, mtx_size);
    }
}