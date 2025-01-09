// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#include <algorithm>
#include <complex>
#include <iostream>
#include <stdexcept>
#include <vector>

// string formatting for exceptions
#include <fmt/format.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "utils/utils.hpp"

using namespace boost::python;
using namespace boost::python::numpy;

/*
positive power-of-2 decompose
represent number as a sum of powers of 2, basically binary notation
e.g. sumOfPo2s(15) = {8, 4, 2, 1}
*/
std::vector<int> sumOfPo2s(int num) {
  std::vector<int> po2s;
  while (num > 0) {
    int po2 = std::log2(num);
    po2s.push_back(1 << po2);
    num -= (1 << po2);
  }
  return po2s;
}

bool is_power_of_two(int num) { return ((num & (num - 1)) == 0) and num != 0; }

int num_bits(int num) {
  int exponent = 1;
  while (num >>= 1)
    exponent++;
  return exponent;
}

int last_power_of_two(int num) { return 1 << (num_bits(num) - 1); }

int next_power_of_two(int num) { return 1 << num_bits(num); }

/*
positive and negative power-of-2 decompose
e.g. po2Decompose(15) = {16, -1}
*/
std::vector<int> po2Decompose(int num) {
  std::vector<int> elts;

  if (num == 0) {
    return elts;
  }

  if (num < 0) {
    elts = po2Decompose(-num);
    std::transform(elts.begin(), elts.end(), elts.begin(),
                   [](int elt) { return -elt; });
    return elts;
  }

  if (is_power_of_two(num)) {
    elts.push_back(num);
    return elts;
  }

  int lower = last_power_of_two(num);
  int upper = next_power_of_two(num);

  int lower_diff = num - lower;
  int upper_diff = upper - num;

  std::vector<int> lower_sum = po2Decompose(lower_diff);
  std::vector<int> upper_sum = po2Decompose(upper_diff);

  if (lower_sum.size() <= upper_sum.size()) {
    lower_sum.push_back(lower);
    return lower_sum;
  } else {
    std::transform(upper_sum.begin(), upper_sum.end(), upper_sum.begin(),
                   [](int elt) { return -elt; });
    upper_sum.push_back(upper);
    return upper_sum;
  }
}