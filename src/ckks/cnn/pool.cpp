// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#include "ckks/cnn/pool.hpp"
#include "ckks/CKKS_ciphertext_extension.hpp"

using namespace pyOpenFHE;
using namespace pyOpenFHE_CKKS;
using namespace boost::python;
using namespace boost::python::numpy;

/*
* Applies a stride-1 convolution with a kernel of 1s, and cyclic rotations (instead of logical).
* The masking and dividing by 4 is accomplished by the downsample masks later,
* so we can get away with a very simple convolution here.
*/ 
void pool_pre_convolution(std::vector<pyOpenFHE_CKKS::CKKSCiphertext>& shards, int num_cols) {
    int num_input_shards = shards.size();

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> shifts(num_input_shards * 4);
    int shift_vals[] = {0, 1, num_cols, num_cols + 1};

    #pragma omp parallel for collapse(2)
    for (int i = 0 ; i < num_input_shards; ++i) {
        for (int j = 0; j < 4; ++j) {
            shifts[i * 4 + j] = shards[i] << shift_vals[j];
        }
    }

    #pragma omp parallel for
    for (int i = 0; i < num_input_shards; ++i) {
        shards[i] = shifts[i * 4];
        for (int j = 1; j < 4; ++j) {
            shards[i] += shifts[i * 4 + j];
        }
    }
}

void pool_horizontal_reduce(std::vector<pyOpenFHE_CKKS::CKKSCiphertext>& shards, int num_rows, int num_cols, int num_physical_channels_per_shard, double fill_value) {
    int num_input_shards = shards.size();
    int shard_size = shards[0].getBatchSize();

    const int half_num_cols = num_cols / 2;
    const int channel_size = num_rows * num_cols;

    std::vector<std::vector<double>> horizontal_masks(half_num_cols, std::vector<double>(shard_size));
    #pragma omp parallel for
    for (int i = 0 ; i < half_num_cols; ++i) { // column index, also vector index
    	for (int j = 0; j < num_rows; ++j) { // row index 
    		for (int k = 0; k < num_physical_channels_per_shard; ++k) { // channel index
    			horizontal_masks[i][i + j * num_cols + k * channel_size] = fill_value;
    		}
    	}
    }

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> horizontal_reductions(half_num_cols * num_input_shards);
    #pragma omp parallel for collapse(2)
    for (int s = 0 ; s < num_input_shards; ++s) {
        for (int i = 0 ; i < half_num_cols; ++i) {
            // if performed sequentially, we could have a loop with `shards[s] <<= 1`
            horizontal_reductions[i + s * half_num_cols] = (shards[s] << i) * horizontal_masks[i];
        }
    }

    #pragma omp parallel for
    for (int s = 0 ; s < num_input_shards; ++s) {
        auto ctxt = horizontal_reductions[s * half_num_cols];
        for (int i = 1 ; i < half_num_cols; ++i) {
            ctxt += horizontal_reductions[i + s * half_num_cols];
        }
        shards[s] = ctxt;
    }
}


void pool_vertical_reduce_image_sharded(std::vector<pyOpenFHE_CKKS::CKKSCiphertext>& shards, int num_rows, int num_cols, int num_physical_channels_per_shard, double fill_value) {
    int num_input_shards = shards.size();
    int shard_size = shards[0].getBatchSize();

    const int half_num_rows = num_rows / 2;
    const int channel_size = num_rows * num_cols;

    std::vector<std::vector<double>> vertical_masks(half_num_rows, std::vector<double>(shard_size));
    #pragma omp parallel for
    for (int i = 0; i < half_num_rows; ++i) { // row index, also vector index
    	for (int j = 0; j < num_cols; ++j) { // column index
    		for (int k = 0; k < num_physical_channels_per_shard; ++k) { // channel index
    			vertical_masks[i][i * half_num_rows + j + k * channel_size] = fill_value;
    		}
    	}
    }

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> vertical_reductions(half_num_rows * num_input_shards);
    #pragma omp parallel for collapse(2)
    for (int s = 0 ; s < num_input_shards; ++s) {
        for (int i = 0 ; i < half_num_rows; ++i) {
            // if performed sequentially, we could have a loop with `shards[s] <<= 3 * half_num_rows`
            vertical_reductions[i + s * half_num_rows] = (shards[s] << (i * half_num_rows * 3)) * vertical_masks[i];
        }
    }

    #pragma omp parallel for
    for (int s = 0 ; s < num_input_shards; ++s) {
        auto ctxt = vertical_reductions[s * half_num_rows];
        for (int i = 1 ; i < half_num_rows; ++i) {
            ctxt += vertical_reductions[i + s * half_num_rows];
        }
        shards[s] = ctxt;
    }

}

void pool_vertical_reduce_channel_sharded(std::vector<pyOpenFHE_CKKS::CKKSCiphertext>& shards, int num_rows, int num_cols) {
    int num_input_shards = shards.size();
    int shard_size = shards[0].getBatchSize();

    const int half_num_rows = num_rows / 2;
    const int half_num_cols = num_cols / 2;

    std::vector<std::vector<double>> vertical_masks(half_num_rows, std::vector<double>(shard_size));
    #pragma omp parallel for
    for (int i = 0; i < half_num_rows; ++i) { // row index, also vector index
    	for (int j = 0; j < half_num_cols; ++j) { // column index
    		vertical_masks[i][i * half_num_cols + j] = 1;
    	}
    }

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> vertical_reductions(half_num_rows * num_input_shards);
    #pragma omp parallel for collapse(2)
    for (int s = 0 ; s < num_input_shards; ++s) {
        for (int i = 0 ; i < half_num_rows; ++i) {
            // if performed sequentially, we could have a loop with `shards[s] <<= 3 * half_num_rows`
            vertical_reductions[i + s * half_num_rows] = (shards[s] << (i * half_num_cols * 3)) * vertical_masks[i];
        }
    }

    #pragma omp parallel for
    for (int s = 0 ; s < num_input_shards; ++s) {
        auto ctxt = vertical_reductions[s * half_num_rows];
        for (int i = 1 ; i < half_num_rows; ++i) {
            ctxt += vertical_reductions[i + s * half_num_rows];
        }
        shards[s] = ctxt;
    }
}

std::vector<pyOpenFHE_CKKS::CKKSCiphertext> pool_consolidate_and_duplicate_image_sharded(std::vector<pyOpenFHE_CKKS::CKKSCiphertext>& shards, int num_rows, int num_cols, int num_physical_channels_per_shard) {
    int num_input_shards = shards.size();

    const int half_num_rows = num_rows / 2;
    const int half_num_cols = num_cols / 2;

    int num_output_shards;
    int new_duplication_ratio; // how many times will we re-duplicate the data?
    switch (num_input_shards) {
        case 1:
            num_output_shards = 1;
            new_duplication_ratio = 4;
            break;
        case 2:
            num_output_shards = 1;
            new_duplication_ratio = 2;
            break;
        default:
            num_output_shards = num_input_shards / 4;
            new_duplication_ratio = 1;
    }

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> output_shards(num_output_shards);

    if (new_duplication_ratio == 1) {
        #pragma omp parallel for
        for (int s = 0; s < num_input_shards; ++s) {
            int r = (s % 4) * half_num_rows * half_num_cols;
            shards[s] >>= r;
        }

        #pragma omp parallel for
        for (int s = 0; s < num_input_shards; s += 4) {
            for (int j = 1; j < 4; ++j) {
                shards[s] += shards[s+j];
            }
            output_shards[s / 4] = shards[s];
        }

        // no duplication step.

    }
    if (new_duplication_ratio == 2) {
        // this is simplified because we know we must have num_input_shards = 2 .
        int r = half_num_rows * half_num_cols;
        
        shards[1] >>= (2 * r);
        shards[0] += shards[1];
        shards[0] += shards[0] >> r;

        output_shards[0] = shards[0];
    }
    if (new_duplication_ratio == 4) {
        // this is simplified because we know we must have num_input_shards = 1 .
        int r = half_num_rows * half_num_cols;
        shards[0] += shards[0] >> r;
        shards[0] += shards[0] >> (2 * r);

        output_shards[0] = shards[0];
    }

    return output_shards;
}

std::vector<pyOpenFHE_CKKS::CKKSCiphertext> pool_consolidate_and_duplicate_channel_sharded(std::vector<pyOpenFHE_CKKS::CKKSCiphertext>& shards) {
    int num_input_shards = shards.size();
    int shard_size = shards[0].getBatchSize();

    int num_output_shards;
    int duplication_ratio;
    if (num_input_shards > 2) {
        num_output_shards = num_input_shards / 4;
        duplication_ratio = 1;
    } else {
        // Degenerate case we have only one channel consisting of only 2 shards.
        // We're then forced to duplicate to fill up the resulting single shard.
        num_output_shards = 1;
        duplication_ratio = 2;
    }

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> output_shards(num_output_shards);

    if (duplication_ratio == 1) {
        int rotation_factor = shard_size / 4;
        for (int output_shard_index = 0; output_shard_index < num_output_shards; output_shard_index++) {
            output_shards[output_shard_index] = shards[output_shard_index * 4];
            // each new shard is four old shards
            for (int i = 1; i < 4; i++) {
                output_shards[output_shard_index] += shards[i + output_shard_index * 4] >> (i * rotation_factor);
            }
        }
    } else {
        int rotation_factor = shard_size / 4;
        output_shards[0] = shards[0] + (shards[1] >> rotation_factor);

        // duplication step
        output_shards[0] = output_shards[0] + (output_shards[0] >> (rotation_factor * 2));
    }

    return output_shards;
}

boost::python::list pool_image_sharded(std::vector<pyOpenFHE_CKKS::CKKSCiphertext>& shards, int mtx_size, bool conv) {
    int shard_size = shards[0].getBatchSize();
    int channel_size = mtx_size * mtx_size; // assuming square matrices, may want to change this assumption later though
    int num_physical_channels_per_shard = shard_size / channel_size;

    // this toggles whether we actually do a pooling or just the downsample, deshard, and duplicate steps
    double fill_value = 1.0;
    if (conv) {
        pool_pre_convolution(shards, mtx_size);
        fill_value = 0.25;
    }

    pool_horizontal_reduce(shards, mtx_size, mtx_size, num_physical_channels_per_shard, fill_value);
    pool_vertical_reduce_image_sharded(shards, mtx_size, mtx_size, num_physical_channels_per_shard, 1.0);

    auto output_shards = pool_consolidate_and_duplicate_image_sharded(shards, mtx_size, mtx_size, num_physical_channels_per_shard);
    int num_output_shards = output_shards.size();

    boost::python::list res = pyOpenFHE::make_list(num_output_shards);
    // #pragma omp parallel for
    for (int s = 0 ; s < num_output_shards; ++s) {
        res[s] = output_shards[s];
    }

    return res;

}

boost::python::list pool_channel_sharded(std::vector<pyOpenFHE_CKKS::CKKSCiphertext>& shards, int mtx_size, bool conv) {
    int shard_size = shards[0].getBatchSize();
    int channel_size = mtx_size * mtx_size; // assuming square matrices, may want to change this assumption later though
    int shards_per_channel = channel_size / shard_size;
    int num_rows_per_shard = mtx_size / shards_per_channel;
    int num_cols_per_shard = mtx_size;

    // this toggles whether we actually do a pooling or just the downsample, consolidate, and duplicate steps
    double fill_value = 1.0;
    if (conv) {
        pool_pre_convolution(shards, num_cols_per_shard);
        fill_value = 0.25;
    }

    pool_horizontal_reduce(shards, num_rows_per_shard, num_cols_per_shard, 1, fill_value);
    pool_vertical_reduce_channel_sharded(shards, num_rows_per_shard, num_cols_per_shard);

    auto output_shards = pool_consolidate_and_duplicate_channel_sharded(shards);
    int num_output_shards = output_shards.size();

    boost::python::list res = pyOpenFHE::make_list(num_output_shards);
    // #pragma omp parallel for
    for (int s = 0 ; s < num_output_shards; ++s) {
        res[s] = output_shards[s];
    }

    return res;

}

boost::python::list pyOpenFHE_CKKS::pool(const boost::python::list &py_shards, int mtx_size, bool conv) {
    int num_input_shards = len(py_shards);

    std::vector<pyOpenFHE_CKKS::CKKSCiphertext> shards(num_input_shards);

    for (int i = 0 ; i < num_input_shards; ++i) {
        shards[i] = extract<pyOpenFHE_CKKS::CKKSCiphertext>(py_shards[i]);
    }

    int shard_size = shards[0].getBatchSize();
    int channel_size = mtx_size * mtx_size; // assuming square matrices, may want to change this assumption later though

    if (shard_size >= channel_size) {
        return pool_image_sharded(shards, mtx_size, conv);
    } else {
        return pool_channel_sharded(shards, mtx_size, conv);
    }
}