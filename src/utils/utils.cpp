// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

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

/// @brief Construct list with `n` elements.  each element is a copy
///        of `item`.
/// @param n Initial container size.
/// @param item Item with which to fill the container.
boost::python::list pyOpenFHE::make_list(const std::size_t n,
                                         boost::python::object item) {

  // >>> [None] * n;
  boost::python::list result;
  result.append(item);
  result *= n;
  return result;
}

list pyOpenFHE::cppVectorToPythonList(const std::vector<double> &vector) {
  boost::python::list pythonList;
  for (unsigned int i = 0; i < vector.size(); i++) {
    pythonList.append(vector[i]);
  }
  return pythonList;
}

list pyOpenFHE::cppLongIntVectorToPythonList(
    const std::vector<int64_t> &vector) {
  boost::python::list pythonList;
  for (unsigned int i = 0; i < vector.size(); i++) {
    pythonList.append(vector[i]);
  }
  return pythonList;
}

ndarray
pyOpenFHE::cppDoubleVectorToNumpyList(const std::vector<double> &vector) {
  Py_intptr_t shape[1] = {(Py_intptr_t)vector.size()};

  boost::python::numpy::ndarray npList =
      boost::python::numpy::zeros(1, shape, dtype::get_builtin<double>());
  for (unsigned int i = 0; i < vector.size(); i++) {
    npList[i] = vector[i];
  }
  return npList;
}

ndarray
pyOpenFHE::cppLongIntVectorToNumpyList(const std::vector<int64_t> &vector) {
  Py_intptr_t shape[1] = {(Py_intptr_t)vector.size()};

  boost::python::numpy::ndarray npList =
      boost::python::numpy::zeros(1, shape, dtype::get_builtin<int64_t>());
  for (unsigned int i = 0; i < vector.size(); i++) {
    npList[i] = vector[i];
  }
  return npList;
}

std::vector<int> pyOpenFHE::pythonListToCppIntVector(const list &pylist) {
  std::vector<int> cppVector;
  for (unsigned int i = 0; i < len(pylist); i++) {
    cppVector.push_back(extract<int>(pylist[i]));
  }
  return cppVector;
}

std::vector<int> pyOpenFHE::numpyListToCppIntVector(const ndarray &nplist) {
  std::vector<int> cppVector;
  for (unsigned int i = 0; i < nplist.shape(0); i++) {
    cppVector.push_back(extract<int>(nplist[i]));
  }
  return cppVector;
}

std::vector<int64_t>
pyOpenFHE::pythonListToCppLongIntVector(const list &pylist) {
  std::vector<int64_t> cppVector;
  for (unsigned int i = 0; i < len(pylist); i++) {
    cppVector.push_back(extract<int64_t>(pylist[i]));
  }
  return cppVector;
}

std::vector<int64_t>
pyOpenFHE::numpyListToCppLongIntVector(const ndarray &nplist) {
  std::vector<int64_t> cppVector;
  for (unsigned int i = 0; i < nplist.shape(0); i++) {
    cppVector.push_back(extract<int64_t>(nplist[i]));
  }
  return cppVector;
}

std::vector<double>
pyOpenFHE::numpyListToCppDoubleVector(const ndarray &nplist) {
  std::string dtype(extract<char const *>(str(nplist.get_dtype())));

  int dimensions = nplist.get_nd();
  if (dimensions != 1) {
    throw std::runtime_error(
        fmt::format("Numpy array must be one-dimensional but had dimension: {}",
                    dimensions));
  }

  std::vector<double> cppVector;
  if (dtype == "float64") {
    for (unsigned int i = 0; i < nplist.shape(0); i++) {
      cppVector.push_back(extract<double>(nplist[i]));
    }
    return cppVector;
  }
  if (dtype == "float32") {
    for (unsigned int i = 0; i < nplist.shape(0); i++) {
      cppVector.push_back((double)extract<float>(nplist[i]));
    }
    return cppVector;
  }
  if (dtype == "int64") {
    for (unsigned int i = 0; i < nplist.shape(0); i++) {
      cppVector.push_back((double)extract<long int>(nplist[i]));
    }
    return cppVector;
  }
  if (dtype == "int32") {
    for (unsigned int i = 0; i < nplist.shape(0); i++) {
      cppVector.push_back((double)extract<int>(nplist[i]));
    }
    return cppVector;
  }

  throw std::runtime_error(
      fmt::format("Unsupported dtype for converting to float64: {}", dtype));
}

pyOpenFHE::boost_vector2d
pyOpenFHE::numpyArrayToCppArray2D(const ndarray &nplist) {

  std::string dtype(extract<char const *>(str(nplist.get_dtype())));

  int dimensions = nplist.get_nd();
  if (dimensions != 2) {
    throw std::runtime_error(fmt::format(
        "Numpy array must be four-dimensional but had dimension: {}",
        dimensions));
  }

  pyOpenFHE::boost_vector2d cppVector(
      boost::extents[nplist.shape(0)][nplist.shape(1)]);

  if (dtype == "float64") {
    for (unsigned int i0 = 0; i0 < nplist.shape(0); i0++) {
      for (unsigned int i1 = 0; i1 < nplist.shape(1); i1++) {
        cppVector[i0][i1] = extract<double>(nplist[i0][i1]);
      }
    }
    return cppVector;
  }
  if (dtype == "float32") {
    for (unsigned int i0 = 0; i0 < nplist.shape(0); i0++) {
      for (unsigned int i1 = 0; i1 < nplist.shape(1); i1++) {
        cppVector[i0][i1] = extract<float>(nplist[i0][i1]);
      }
    }
    return cppVector;
  }
  if (dtype == "int64") {
    for (unsigned int i0 = 0; i0 < nplist.shape(0); i0++) {
      for (unsigned int i1 = 0; i1 < nplist.shape(1); i1++) {
        cppVector[i0][i1] = (double)extract<long int>(nplist[i0][i1]);
      }
    }
    return cppVector;
  }
  if (dtype == "int32") {
    for (unsigned int i0 = 0; i0 < nplist.shape(0); i0++) {
      for (unsigned int i1 = 0; i1 < nplist.shape(1); i1++) {
        cppVector[i0][i1] = (double)extract<int>(nplist[i0][i1]);
      }
    }
    return cppVector;
  }

  throw std::runtime_error(
      fmt::format("Unsupported dtype for converting to float64: {}", dtype));
}

pyOpenFHE::boost_vector4d
pyOpenFHE::numpyArrayToCppArray4D(const ndarray &nplist) {

  std::string dtype(extract<char const *>(str(nplist.get_dtype())));

  int dimensions = nplist.get_nd();
  if (dimensions != 4) {
    throw std::runtime_error(fmt::format(
        "Numpy array must be four-dimensional but had dimension: {}",
        dimensions));
  }

  pyOpenFHE::boost_vector4d cppVector(boost::extents[nplist.shape(
      0)][nplist.shape(1)][nplist.shape(2)][nplist.shape(3)]);

  if (dtype == "float64") {
    for (unsigned int i0 = 0; i0 < nplist.shape(0); i0++) {
      for (unsigned int i1 = 0; i1 < nplist.shape(1); i1++) {
        for (unsigned int i2 = 0; i2 < nplist.shape(2); i2++) {
          for (unsigned int i3 = 0; i3 < nplist.shape(3); i3++) {
            cppVector[i0][i1][i2][i3] = extract<double>(nplist[i0][i1][i2][i3]);
          }
        }
      }
    }
    return cppVector;
  }
  if (dtype == "float32") {
    for (unsigned int i0 = 0; i0 < nplist.shape(0); i0++) {
      for (unsigned int i1 = 0; i1 < nplist.shape(1); i1++) {
        for (unsigned int i2 = 0; i2 < nplist.shape(2); i2++) {
          for (unsigned int i3 = 0; i3 < nplist.shape(3); i3++) {
            cppVector[i0][i1][i2][i3] = extract<float>(nplist[i0][i1][i2][i3]);
          }
        }
      }
    }
    return cppVector;
  }
  if (dtype == "int64") {
    for (unsigned int i0 = 0; i0 < nplist.shape(0); i0++) {
      for (unsigned int i1 = 0; i1 < nplist.shape(1); i1++) {
        for (unsigned int i2 = 0; i2 < nplist.shape(2); i2++) {
          for (unsigned int i3 = 0; i3 < nplist.shape(3); i3++) {
            cppVector[i0][i1][i2][i3] =
                (double)extract<long int>(nplist[i0][i1][i2][i3]);
          }
        }
      }
    }
    return cppVector;
  }
  if (dtype == "int32") {
    for (unsigned int i0 = 0; i0 < nplist.shape(0); i0++) {
      for (unsigned int i1 = 0; i1 < nplist.shape(1); i1++) {
        for (unsigned int i2 = 0; i2 < nplist.shape(2); i2++) {
          for (unsigned int i3 = 0; i3 < nplist.shape(3); i3++) {
            cppVector[i0][i1][i2][i3] =
                (double)extract<int>(nplist[i0][i1][i2][i3]);
          }
        }
      }
    }
    return cppVector;
  }

  throw std::runtime_error(
      fmt::format("Unsupported dtype for converting to float64: {}", dtype));
}

std::vector<double> pyOpenFHE::pythonListToCppDoubleVector(const list &pylist) {
  std::vector<double> cppVector;
  for (unsigned int i = 0; i < len(pylist); i++) {
    cppVector.push_back(extract<double>(pylist[i]));
  }
  return cppVector;
}

// tile a vector to fill out a max size
// i.e. tileVector([1, 2], 16) gives
// [1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2]
// i.e. vector with m elements becomes vector with final_size elements
// useful for resizing vectors before encrypting
void tileVector(std::vector<double> &vals, unsigned int final_size) {
  auto current_size = vals.size();
  // reserve new size, lets us use copy
  vals.resize(final_size);
  while (current_size < final_size) {
    // the end() reference is slippery, just use relative to begin()
    std::copy_n(vals.begin(), current_size, vals.begin() + current_size);
    current_size <<= 1; // double, but cooler
  }
}

void tileVector(std::vector<int64_t> &vals, unsigned int final_size) {
  auto current_size = vals.size();
  // reserve new size, lets us use copy
  vals.resize(final_size);
  while (current_size < final_size) {
    // the end() reference is slippery, just use relative to begin()
    std::copy_n(vals.begin(), current_size, vals.begin() + current_size);
    current_size <<= 1; // double, but cooler
  }
}

void tileVector(std::vector<int> &vals, unsigned int final_size) {
  auto current_size = vals.size();
  // reserve new size, lets us use copy
  vals.resize(final_size);
  while (current_size < final_size) {
    // the end() reference is slippery, just use relative to begin()
    std::copy_n(vals.begin(), current_size, vals.begin() + current_size);
    current_size <<= 1; // double, but cooler
  }
}

template <typename T> void print_vector(std::vector<T> vec) {
  for (int i = 0; i < (int)vec.size(); i++) {
    std::cout << vec[i] << " ";
  }
  std::cout << std::endl;
}