// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef OpenFHE_PYTHON_UTILS_H
#define OpenFHE_PYTHON_UTILS_H

#include <complex>
#include <vector>

#include "boost/multi_array.hpp"
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

using namespace boost::python;
using namespace boost::python::numpy;

namespace pyOpenFHE {

// TODO do these really belong in utils.hpp rather than he-cnn.hpp?
// for kernel slicing
typedef typename boost::multi_array<double, 2> boost_vector2d;
typedef typename boost::multi_array<double, 4> boost_vector4d;
typedef boost::multi_array_types::index_range srange;
typedef typename boost_vector4d::array_view<4>::type boost_vector4d_slice;

boost::python::list
make_list(const std::size_t n,
          boost::python::object item = boost::python::object() /* none */);

// converts std::vector to python list, O(n) time
list cppVectorToPythonList(const std::vector<double> &);

list cppLongIntVectorToPythonList(const std::vector<int64_t> &vector);

// O(n) std::vector to numpy array
// could be faster if we didn't have use complex<double>
// in that case we could just copy the vector contents to the numpy array
// that's probably optimized or something, better than iteration
ndarray cppDoubleVectorToNumpyList(const std::vector<double> &);

ndarray cppLongIntVectorToNumpyList(const std::vector<int64_t> &vector);

// these two are used for converting lists of indices for EvalAtIndexKeyGen
// since they have to be ints, and we're on the static typing side of things
std::vector<int> pythonListToCppIntVector(const list &);

std::vector<int> numpyListToCppIntVector(const ndarray &);

boost_vector4d numpyArrayToCppArray4D(const ndarray &nplist);
boost_vector2d numpyArrayToCppArray2D(const ndarray &nplist);

// conversion for complex<double>
// for numpy arrays, the dtype must be specified to float/double or else C++
// will throw a fit
std::vector<double> numpyListToCppDoubleVector(const ndarray &);

std::vector<double> pythonListToCppDoubleVector(const list &);

std::vector<int64_t> pythonListToCppLongIntVector(const list &pylist);

std::vector<int64_t> numpyListToCppLongIntVector(const ndarray &nplist);

} // namespace pyOpenFHE

std::vector<int> sumOfPo2s(int);

void tileVector(std::vector<double> &vals, unsigned int n);
void tileVector(std::vector<int64_t> &vals, unsigned int n);
void tileVector(std::vector<int> &vals, unsigned int n);

template <typename T> void print_vector(std::vector<T> vec);

#endif /* OpenFHE_PYTHON_UTILS_H */