// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#include "ckks/CKKS_ciphertext_extension.hpp"
#include "ckks/cnn/he_cnn.hpp"
#include "utils/utils.hpp"

using namespace pyOpenFHE;
using namespace boost::python;
using namespace boost::python::numpy;

// this could be reworked, but not a priority since it works
int kernel_index_to_shift(int i, int ker_size) {
    int shift;
    if ((ker_size % 2) == 0) {
        shift = i - (ker_size / 2) + 1;
    } else {
        shift = i - (ker_size / 2);
    }
    return shift;
}

int shift_to_kernel_index(int shift, int ker_size) {
    for (int i = 0; i < ker_size; i++) {
        if (kernel_index_to_shift(i, ker_size) == shift) {
            return i;
        }
    }
    throw std::invalid_argument("test");
}

// mask gen
std::vector<int> generate_up_mask(int mtx_num_rows, int mtx_num_cols, int num_shift_up) {
    std::vector<int> mask(mtx_num_rows * mtx_num_cols);
    std::iota(std::begin(mask), std::end(mask), 0);
    std::transform(mask.begin(), mask.end(), mask.begin(), [=](int i){
        if (i / mtx_num_cols >= (mtx_num_rows - num_shift_up)) {
            return 0;
        } else {
            return 1;
        }
    });
    return mask;
}

std::vector<int> generate_down_mask(int mtx_num_rows, int mtx_num_cols, int num_shift_down) {
    std::vector<int> mask(mtx_num_rows * mtx_num_cols);
    std::iota(std::begin(mask), std::end(mask), 0);
    std::transform(mask.begin(), mask.end(), mask.begin(), [=](int i){
        if (i / mtx_num_cols < num_shift_down) {
            return 0;
        } else {
            return 1;
        }
    });
    return mask;
}

std::vector<int> generate_left_mask(int mtx_num_rows, int mtx_num_cols, int num_shift_left) {
    std::vector<int> mask(mtx_num_rows * mtx_num_cols);
    std::iota(std::begin(mask), std::end(mask), 0);
    std::transform(mask.begin(), mask.end(), mask.begin(), [=](int i){
        if (i % mtx_num_cols >= (mtx_num_cols - num_shift_left)) {
            return 0;
        } else {
            return 1;
        }
    });
    return mask;
}

std::vector<int> generate_right_mask(int mtx_num_rows, int mtx_num_cols, int num_shift_right) {
    std::vector<int> mask(mtx_num_rows * mtx_num_cols);
    std::iota(std::begin(mask), std::end(mask), 0);
    std::transform(mask.begin(), mask.end(), mask.begin(), [=](int i){
        if (i % mtx_num_cols < num_shift_right) {
            return 0;
        } else {
            return 1;
        }
    });
    return mask;
}

std::vector<int> generate_ud_mask(int mtx_num_rows, int mtx_num_cols, int num_shift_up) {
    if (num_shift_up < 0) {
        return generate_down_mask(mtx_num_rows, mtx_num_cols, -num_shift_up);
    } else {
        return generate_up_mask(mtx_num_rows, mtx_num_cols, num_shift_up);
    }
}

std::vector<int> generate_lr_mask(int mtx_num_rows, int mtx_num_cols, int num_shift_left) {
    if (num_shift_left < 0) {
        return generate_right_mask(mtx_num_rows, mtx_num_cols, -num_shift_left);
    } else {
        return generate_left_mask(mtx_num_rows, mtx_num_cols, num_shift_left);
    }
}

std::vector<int> make_shift_mask_image_sharded(int num_mtxs, int mtx_num_rows, int mtx_num_cols, int num_shift_up, int num_shift_left) {
    std::vector<int> ud_mask = generate_ud_mask(mtx_num_rows, mtx_num_cols, num_shift_up);
    std::vector<int> lr_mask = generate_lr_mask(mtx_num_rows, mtx_num_cols, num_shift_left);

    int mtx_area = mtx_num_rows * mtx_num_cols;
    int batch_size = num_mtxs * mtx_area;

    // AND of the two masks
    std::vector<int> mask(mtx_area);
    for (int i = 0; i < mtx_area; i++) {
        mask[i] = ud_mask[i] & lr_mask[i];
    }
    
    // tile the mask using std::copy
    tileVector(mask, batch_size);

    return mask;
}

std::vector<int> make_shift_mask_channel_shard(int num_rows, int num_cols, int num_shift_up, int num_shift_left) {
    std::vector<int> ud_mask = generate_ud_mask(num_rows, num_cols, num_shift_up);
    std::vector<int> lr_mask = generate_lr_mask(num_rows, num_cols, num_shift_left);

    int area = num_rows * num_cols;

    // AND of the two masks
    std::vector<int> mask(area);
    for (int i = 0; i < area; i++) {
        mask[i] = ud_mask[i] & lr_mask[i];
    }

    return mask;
}

std::vector<int> make_shift_mask_bleed_channel_shard(int num_rows, int num_cols, int num_shift_up, int num_shift_left) {
    // only the up-down mask should be inverted for bleed shards
    std::vector<int> ud_mask = generate_ud_mask(num_rows, num_cols, num_shift_up);
    std::vector<int> lr_mask = generate_lr_mask(num_rows, num_cols, num_shift_left);

    int area = num_rows * num_cols;

    // AND of the two masks
    std::vector<int> mask(area);
    for (int i = 0; i < area; i++) {
        mask[i] = (1 - ud_mask[i]) & lr_mask[i];
    }

    return mask;
}

// template <typename T>
void print_vector(std::vector<int> vec) {
    for (int i = 0; i < (int) vec.size(); i++) {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;
}

void print_vector(std::vector<double> vec) {
    for (int i = 0; i < (int) vec.size(); i++) {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;
}

void print_mask(std::vector<int> vec, int num_rows, int num_cols) {
    for (int i = 0; i < num_rows; i++) {
        for (int j = 0; j < num_cols; j++) {
            std::cout << vec[i * num_cols + j];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void print_mask(std::vector<double> vec, int num_rows, int num_cols) {
    for (int i = 0; i < num_rows; i++) {
        for (int j = 0; j < num_cols; j++) {
            std::cout << vec[i * num_cols + j];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}