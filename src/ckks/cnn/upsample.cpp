// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#include "ckks/CKKS_ciphertext_extension.hpp"
#include "ckks/cnn/upsample.hpp"

#include <stdexcept>
#include <fmt/format.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/scope.hpp>
#include <omp.h>
#include <cstdlib>


std::vector<pyOpenFHE_CKKS::CKKSCiphertext> upsample_vertical_expand(const std::vector<pyOpenFHE_CKKS::CKKSCiphertext>& shards, int num_rows, int num_cols, int num_physical_channels_per_shard, int duplication_ratio, double fill_value) {
    int num_input_shards = shards.size();
    int shard_size = shards[0].getBatchSize();

    const int channel_size = num_rows * num_cols;
    const int channel_size_after_upsample = channel_size * 4;
    /*
    distance_to_next_subchannel:
        if output is small shards and have no duplication, this is the start of the 2nd physical channel.
        if have duplication, this is the start of either the 2nd or 4th physical channel.
        if output is big shards, this is either 1/4 or 1/2 way through the 1st channel.
        if input is big shards, this is 1/4 of the way through the first shard.

    num_rows_per_shard_after_upsample:
        this counts the number of pre-upsample rows per channel that fit into a single ctxt,
        i.e. we don't count the zero-filled rows that we're inserting.
        it is only really used if we output big shards.
        if we output small shards then it equals num_rows
    */
    int num_rows_per_shard_after_upsample = num_rows;
    int distance_to_next_subchannel = channel_size;
    int num_shifts_per_shard = 4;
    if(channel_size_after_upsample > shard_size) {
        num_rows_per_shard_after_upsample = num_rows / (channel_size_after_upsample / shard_size);
        distance_to_next_subchannel = channel_size / (channel_size_after_upsample / shard_size);

        // complete edge case, for when you have exactly one shard duplicated twice of half the shard size
        if(duplication_ratio == 2) {
            num_shifts_per_shard = 2;
        }
    }
    else {
        if(duplication_ratio == 2) {
            distance_to_next_subchannel = 2 * channel_size;
            num_shifts_per_shard = 2;
        }
        if(duplication_ratio > 2) {
            distance_to_next_subchannel = 4 * channel_size;
            num_shifts_per_shard = 1;
        }
    }

    int num_expanded_shards = num_input_shards * num_shifts_per_shard;

    if(num_rows_per_shard_after_upsample < 1) { // not a typo.
        std::string s = fmt::format("Shards must be able to store at least two rows");
        throw std::runtime_error(s);
    }

    std::vector<std::vector<double>> vertical_masks(num_rows_per_shard_after_upsample, std::vector<double>(shard_size));
    #pragma omp parallel for
    for (int j = 0; j < num_rows_per_shard_after_upsample; ++j) { // row index 
        for (int i = 0 ; i < num_cols; ++i) { // column index
    		for (int k = 0; k < num_physical_channels_per_shard; k += 4) { // channel index
    			vertical_masks[j][i + j * num_cols * 4 + k * channel_size] = fill_value;
    		}
    	}
    }

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> shifted_shards(num_expanded_shards);
    #pragma omp parallel for collapse(2)
    for (int s = 0 ; s < num_input_shards; ++s) {
        for(int i = 0; i < num_shifts_per_shard; ++i) {
            shifted_shards[i + s * num_shifts_per_shard] = shards[s] << (i * distance_to_next_subchannel);
        }
    }

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> vertical_expansions(num_rows_per_shard_after_upsample * num_expanded_shards);
    #pragma omp parallel for collapse(2)
    for (int s = 0 ; s < num_expanded_shards; ++s) {
        for (int i = 0 ; i < num_rows_per_shard_after_upsample; ++i) {
            // if performed sequentially, we could have a loop with `shifted_shards[s] >>= 3 * num_cols`
            vertical_expansions[i + s * num_rows_per_shard_after_upsample] = (shifted_shards[s] >> (3 * num_cols * i)) * vertical_masks[i];
        }
    }

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> new_shards(num_expanded_shards);
    #pragma omp parallel for
    for (int s = 0; s < num_expanded_shards; ++s) {
        auto ctxt = vertical_expansions[s * num_rows_per_shard_after_upsample];
        for (int i = 1 ; i < num_rows_per_shard_after_upsample; ++i) {
            ctxt += vertical_expansions[i + s * num_rows_per_shard_after_upsample];
        }
        new_shards[s] = ctxt;
    }

    return new_shards;

}

/*
Note: the channel dimensions in this function are a little strange,
because this is called after the upsample_vertical_expand() function
has already done the bulk of the work in reshaping the inputs.
*/
void upsample_horizontal_expand(std::vector<pyOpenFHE_CKKS::CKKSCiphertext>& shards, int num_rows, int num_cols, double fill_value) {
    int num_input_shards = shards.size();
    int shard_size = shards[0].getBatchSize();

    const int channel_size = num_rows * num_cols;
    const int channel_size_after_upsample = channel_size * 4;
    int num_physical_channels_per_shard = shard_size / channel_size;
    if(num_physical_channels_per_shard == 0) {
        num_physical_channels_per_shard = 1;
    }

    int num_rows_per_shard_after_upsample = num_rows;
    if(channel_size_after_upsample > shard_size) {
        num_rows_per_shard_after_upsample = num_rows / (channel_size_after_upsample / shard_size);
    }

    std::vector<std::vector<double>> horizontal_masks(num_cols, std::vector<double>(shard_size));
    #pragma omp parallel for
    for (int i = 0; i < num_cols; ++i) { // column index
        for (int j = 0; j < num_rows_per_shard_after_upsample; ++j) { // row index 
    		for (int k = 0; k < num_physical_channels_per_shard; k += 4) { // channel index
    			horizontal_masks[i][i * 2 + j * num_cols * 4 + k * channel_size] = fill_value;
    		}
    	}
    }

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> horizontal_expansions(num_cols * num_input_shards);
    #pragma omp parallel for collapse(2)
    for (int s = 0 ; s < num_input_shards; ++s) {
        for (int i = 0 ; i < num_cols; ++i) {
            // if performed sequentially, we could have a loop with `shards[s] >>= 1`
            horizontal_expansions[i + s * num_cols] = (shards[s] >> (i)) * horizontal_masks[i];
        }
    }

    #pragma omp parallel for
    for (int s = 0 ; s < num_input_shards; ++s) {
        auto ctxt = horizontal_expansions[s * num_cols];
        for (int i = 1 ; i < num_cols; ++i) {
            ctxt += horizontal_expansions[i + s * num_cols];
        }
        shards[s] = ctxt;
    }

}

/*
go from a 2x2 bed of nails upsample to a 2x2 nearest neighbor upsample
*/
void nearest_neighbor_interpolate(std::vector<pyOpenFHE_CKKS::CKKSCiphertext>& shards, int num_cols) {
    int num_input_shards = shards.size();
    #pragma omp parallel for
    for (int s = 0; s < num_input_shards; ++s) {
        auto ctxt = shards[s];
        ctxt += ctxt >> 1;
        ctxt += ctxt >> num_cols;
        shards[s] = ctxt;
    }
}

boost::python::list small_shards_upsample(const std::vector<pyOpenFHE_CKKS::CKKSCiphertext> & shards, const int mtx_size, const ndarray &permutation, int upsample_type) {

    int shard_size = shards[0].getBatchSize();
    int channel_size = mtx_size * mtx_size; // assuming square matrices, may want to change this assumption later though

    auto sigma = numpyListToCppLongIntVector(permutation);
    int num_logical_channels = sigma.size();
    int num_physical_channels_per_shard = shard_size / channel_size;

    int duplication_ratio = 1;
    if(shard_size > channel_size) {
        duplication_ratio = num_physical_channels_per_shard / num_logical_channels;
    }

    double fill_value = 1.0;
    
    auto output_shards = upsample_vertical_expand(shards, mtx_size, mtx_size, num_physical_channels_per_shard, duplication_ratio, fill_value);
    upsample_horizontal_expand(output_shards, mtx_size, mtx_size, fill_value);

    if(upsample_type == 0) {
        // bed of nails
        // nothing to do here.
    }
    else if(upsample_type == 1) {
        // nearest neighbor
        nearest_neighbor_interpolate(output_shards, mtx_size * 2);
    }
    else {
        std::string s = fmt::format("Upsample type #{} is not supported", upsample_type);
        throw std::runtime_error(s);
    }

    int num_output_shards = output_shards.size();
    boost::python::list res = pyOpenFHE::make_list(num_output_shards);

    for (int s = 0 ; s < num_output_shards; ++s) {
        res[s] = output_shards[s];
    }

    return res;

}

boost::python::list big_shards_upsample(const std::vector<pyOpenFHE_CKKS::CKKSCiphertext> & shards, const int mtx_size, const ndarray &permutation, int upsample_type) {

    int shard_size = shards[0].getBatchSize();

    int num_rows_per_shard = shard_size / mtx_size;

    int num_physical_channels_per_shard = 1;
    int duplication_ratio = 1;
    double fill_value = 1.0;
    
    auto output_shards = upsample_vertical_expand(shards, num_rows_per_shard, mtx_size, num_physical_channels_per_shard, duplication_ratio, fill_value);
    upsample_horizontal_expand(output_shards, num_rows_per_shard, mtx_size, fill_value);

    if(upsample_type == 0) {
        // bed of nails
        // nothing to do here.
    }
    else if(upsample_type == 1) {
        // nearest neighbor
        nearest_neighbor_interpolate(output_shards, mtx_size * 2);
    }
    else {
        std::string s = fmt::format("Upsample type #{} is not supported", upsample_type);
        throw std::runtime_error(s);
    }

    int num_output_shards = output_shards.size();
    boost::python::list res = pyOpenFHE::make_list(num_output_shards);

    for (int s = 0 ; s < num_output_shards; ++s) {
        res[s] = output_shards[s];
    }

    return res;


}


/*
The permutation is passed in because big shards are never permuted.
    If we have small shards before upsampling but big shards after,
    then we rearrange the shards to remove the permutation.
If we output small shards, then we ignore the permutation.

upsample_type:
    = 0 for bed of nails (fill with zeroes)
    = 1 for nearest neighbor
*/
boost::python::list pyOpenFHE_CKKS::upsample(const boost::python::list &py_shards, const int mtx_size, const ndarray &permutation, int upsample_type){

    int num_input_shards = len(py_shards);
    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> shards(num_input_shards);

    for (int i = 0 ; i < num_input_shards; ++i) {
        shards[i] = extract<pyOpenFHE_CKKS::CKKSCiphertext>(py_shards[i]);
    }

    int shard_size = shards[0].getBatchSize();
    int channel_size = mtx_size * mtx_size; // assuming square matrices, may want to change this assumption later though


    if (shard_size >= channel_size) {
        return small_shards_upsample(shards, mtx_size, permutation, upsample_type);

    } else {
        return big_shards_upsample(shards, mtx_size, permutation, upsample_type);
    }

    return py_shards;

}