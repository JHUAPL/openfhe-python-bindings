// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef CKKS_UTILS_H
#define CKKS_UTILS_H

#include <vector>

int kernel_index_to_shift(int i, int ker_size);
int shift_to_kernel_index(int shift, int ker_size);

std::vector<int> generate_up_mask(int mtx_num_rows, int mtx_num_cols, int num_shift_up);
std::vector<int> generate_down_mask(int mtx_num_rows, int mtx_num_cols, int num_shift_down);
std::vector<int> generate_left_mask(int mtx_num_rows, int mtx_num_cols, int num_shift_left);
std::vector<int> generate_right_mask(int mtx_num_rows, int mtx_num_cols, int num_shift_right);
std::vector<int> generate_ud_mask(int mtx_num_rows, int mtx_num_cols, int num_shift_up);
std::vector<int> generate_lr_mask(int mtx_num_rows, int mtx_num_cols, int num_shift_left);
std::vector<int> make_shift_mask_image_sharded(int num_mtxs, int mtx_num_rows, int mtx_num_cols, int num_shift_up, int num_shift_left);
std::vector<int> make_shift_mask_channel_shard(int num_rows, int num_cols, int num_shift_up, int num_shift_left);
std::vector<int> make_shift_mask_bleed_channel_shard(int num_rows, int num_cols, int num_shift_up, int num_shift_left);

void print_vector(std::vector<int> vec);
void print_vector(std::vector<double> vec);
void print_mask(std::vector<int> vec, int num_rows, int num_cols);
void print_mask(std::vector<double> vec, int num_rows, int num_cols);

#endif