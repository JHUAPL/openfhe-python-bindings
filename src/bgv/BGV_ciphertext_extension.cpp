// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#include "openfhe.h"

#include <complex>
#include <string>
#include <vector>

// string formatting for exceptions
#include <fmt/format.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "bgv/BGV_ciphertext_extension.hpp"
#include "bgv/BGV_key_operations.hpp"
#include "utils/rotate_utils.hpp"
#include "utils/utils.hpp"

using namespace lbcrypto;
using namespace boost::python;
using namespace boost::python::numpy;
using namespace pyOpenFHE;

namespace pyOpenFHE_BGV {

BGVCryptoContext BGVCiphertext::getCryptoContext(void) const {
  return BGVCryptoContext(cipher->GetCryptoContext());
}

BGVCiphertext BGVCiphertext::array_ufunc(
    const boost::python::object ufunc, const boost::python::str method,
    const boost::python::numpy::ndarray &vals, const BGVCiphertext &cipher) {
  // this is a little rough but I can't think of a better way
  // supposedly if you set __array_ufunc__ to None, numpy will default to the
  // __rmul__ and __radd__ methods, but it doesn't seem to work with C++
  // extensions or something so we define this to override the default ufunc
  // behavior because this is defined, numpy will call into this instead of
  // ufunc
  std::string op =
      boost::python::extract<std::string>(ufunc.attr("__name__"))();

  if (op == "multiply") {
    return cipher * vals;
  } else if (op == "add") {
    return cipher + vals;
  } else {
    throw pyOpenFHE::not_implemented_exception(
        fmt::format("operator {} between ndarray and BGVCiphertext", op));
  }
}

// a whole load of operators
// we need to specify ALL of these, and then specify them again in the
// bindings...

bool operator==(const BGVCiphertext &c1, const BGVCiphertext &c2) {
  return c1.cipher == c2.cipher;
}

bool operator!=(const BGVCiphertext &c1, const BGVCiphertext &c2) {
  return !(c1 == c2);
}

BGVCiphertext operator+=(BGVCiphertext &c1, const BGVCiphertext &c2) {
  c1.cipher += c2.cipher;
  return c1;
}

BGVCiphertext operator+(BGVCiphertext c1, const BGVCiphertext &c2) {
  return c1 += c2;
}

BGVCiphertext operator-=(BGVCiphertext &c1, const BGVCiphertext &c2) {
  c1.cipher -= c2.cipher;
  return c1;
}

BGVCiphertext operator-(BGVCiphertext c1, const BGVCiphertext &c2) {
  return c1 -= c2;
}

BGVCiphertext operator*=(BGVCiphertext &c1, const BGVCiphertext &c2) {
  // TODO: verify this condition is still the same for BGV
  if ((c1.getTowersRemaining() <= 2) || (c2.getTowersRemaining() <= 2)) {
    throw std::runtime_error(
        fmt::format("Insufficient number of towers remaining to perform a "
                    "multiplication = {}, {}",
                    c1.getTowersRemaining(), c2.getTowersRemaining()));
  }
  c1.cipher *= c2.cipher;
  return c1;
}

BGVCiphertext operator*(BGVCiphertext c1, const BGVCiphertext &c2) {
  return c1 *= c2;
}

/*
uses the positive and negative power-of-2 decomposition
e.g. decompose(15) = {16, -1}
*/
BGVCiphertext EvalRotatePositiveNegativePow2(BGVCiphertext &ctxt, int r) {
  if (r == 0) {
    // no change
    return ctxt;
  } else {
    // cyclic packing
    int N = ctxt.cipher->GetEncodingParameters()->GetBatchSize();

    if (abs(r) > N) {
      throw std::runtime_error(fmt::format(
          "rotation value = {} is too large compared to batch size = {}", r,
          N));
    }

    std::vector<int> po2s = po2Decompose(r);

    for (int i : po2s) {
      ctxt.cipher =
          ctxt.cipher->GetCryptoContext()->EvalAtIndex(ctxt.cipher, i);
    }
    return ctxt;
  }
}

/*
uses the positive power-of-2 decomposition
e.g. decompose(15) = {8, 4, 2, 1}
*/
BGVCiphertext EvalRotatePositivePow2(BGVCiphertext &ctxt, int r) {
  if (r == 0) {
    // no change
    return ctxt;
  } else {
    // cyclic packing
    int N = ctxt.cipher->GetEncodingParameters()->GetBatchSize();

    if (abs(r) > N) {
      throw std::runtime_error(fmt::format(
          "rotation value = {} is too large compared to batch size = {}", r,
          N));
    }

    int mult = (r > 0) ? 1 : -1;
    std::vector<int> po2s = sumOfPo2s(abs(r));

    for (int i : po2s) {
      ctxt.cipher = ctxt.cipher->GetCryptoContext()->EvalAtIndex(
          ctxt.cipher, mult * (1 << i));
    }
    return ctxt;
  }
}

BGVCiphertext BGVRotateEvalAtIndex(BGVCiphertext ctxt, int r) {
  ctxt.cipher = ctxt.cipher->GetCryptoContext()->EvalAtIndex(ctxt.cipher, r);
  return ctxt;
}

BGVCiphertext operator<<=(BGVCiphertext &ctxt, int r) {
  return EvalRotatePositiveNegativePow2(ctxt, r);
}

/*
TODO
need error checking to see if that rotation key actually exists.
Also this function should support integer numpy arrays as well
*/
boost::python::list BGVHoistedRotations(const BGVCiphertext &ctxt,
                                        const boost::python::list &pylist) {
  std::vector<int> rotations = pyOpenFHE::pythonListToCppIntVector(pylist);

  auto cc = ctxt.cipher->GetCryptoContext();
  auto cPrecomp = cc->EvalFastRotationPrecompute(ctxt.cipher);

  // M is the cyclotomic order and we need it to call EvalFastRotation
  uint32_t N = cc->GetRingDimension();
  uint32_t M = 2 * N;

  // initialize list to correct length
  boost::python::object empty_item = boost::python::object(); // None
  boost::python::list result;
  result.append(empty_item);
  result *= rotations.size();

  for (unsigned int i = 0; i < rotations.size(); i++) {
    result[i] = pyOpenFHE_BGV::BGVCiphertext(
        cc->EvalFastRotation(ctxt.cipher, rotations[i], M, cPrecomp));
  }
  return result;
}

BGVCiphertext operator>>=(BGVCiphertext &ctxt, int r) { return ctxt <<= (-r); }

BGVCiphertext operator<<(BGVCiphertext ctxt, int r) { return ctxt <<= r; }

BGVCiphertext operator>>(BGVCiphertext ctxt, int r) { return ctxt >>= r; }

BGVCiphertext operator+=(BGVCiphertext &ctxt, int64_t val) {
  std::vector<int64_t> vals = {val};
  // size_t final_size = ctxt.cipher->GetEncodingParameters()->GetBatchSize();
  size_t final_size = ctxt.cipher->GetCryptoContext()->GetRingDimension() / 2;
  tileVector(vals, final_size);
  auto ptxt = ctxt.cipher->GetCryptoContext()->MakePackedPlaintext(vals);
  ctxt.cipher = ctxt.cipher->GetCryptoContext()->EvalAdd(ctxt.cipher, ptxt);
  return ctxt;
}

BGVCiphertext operator+(BGVCiphertext ctxt, int64_t val) { return ctxt += val; }

BGVCiphertext operator+(int64_t val, BGVCiphertext ctxt) { return ctxt += val; }

BGVCiphertext operator+=(BGVCiphertext &ctxt, std::vector<int64_t> vals) {
  size_t N = ctxt.cipher->GetEncodingParameters()->GetBatchSize();
  if (vals.size() != N) {
    std::string s = fmt::format("Provided vector has length = {}, but the "
                                "CryptoContext batch size = {}",
                                vals.size(), N);
    throw std::runtime_error(s);
  }
  size_t final_size = ctxt.cipher->GetCryptoContext()->GetRingDimension() / 2;
  tileVector(vals, final_size);
  auto ptxt = ctxt.cipher->GetCryptoContext()->MakePackedPlaintext(vals);
  ctxt.cipher = ctxt.cipher->GetCryptoContext()->EvalAdd(ctxt.cipher, ptxt);
  return ctxt;
}

BGVCiphertext operator+=(BGVCiphertext &ctxt,
                         const boost::python::list &pyvals) {
  std::vector<int64_t> vals = pythonListToCppLongIntVector(pyvals);
  return ctxt += vals;
}

BGVCiphertext operator+=(BGVCiphertext &ctxt,
                         const boost::python::numpy::ndarray &pyvals) {
  std::vector<int64_t> vals = numpyListToCppLongIntVector(pyvals);
  return ctxt += vals;
}

BGVCiphertext operator+(BGVCiphertext ctxt, const std::vector<int64_t> &vals) {
  return ctxt += vals;
}

BGVCiphertext operator+(BGVCiphertext ctxt, const boost::python::list &vals) {
  return ctxt += vals;
}

BGVCiphertext operator+(BGVCiphertext ctxt,
                        const boost::python::numpy::ndarray &vals) {
  return ctxt += vals;
}

BGVCiphertext operator+(const std::vector<int64_t> &vals, BGVCiphertext ctxt) {
  return ctxt += vals;
}

BGVCiphertext operator+(const boost::python::list &vals, BGVCiphertext ctxt) {
  return ctxt += vals;
}

BGVCiphertext operator+(const boost::python::numpy::ndarray &vals,
                        BGVCiphertext ctxt) {
  return ctxt += vals;
}

BGVCiphertext operator-=(BGVCiphertext &ctxt, std::vector<int64_t> vals) {
  size_t N = ctxt.cipher->GetEncodingParameters()->GetBatchSize();
  if (vals.size() != N) {
    std::string s = fmt::format("Provided vector has length = {}, but the "
                                "CryptoContext batch size = {}",
                                vals.size(), N);
    throw std::runtime_error(s);
  }
  size_t final_size = ctxt.cipher->GetCryptoContext()->GetRingDimension() / 2;
  tileVector(vals, final_size);
  auto ptxt = ctxt.cipher->GetCryptoContext()->MakePackedPlaintext(vals);
  ctxt.cipher = ctxt.cipher->GetCryptoContext()->EvalSub(ctxt.cipher, ptxt);
  return ctxt;
}

BGVCiphertext operator-=(BGVCiphertext &ctxt,
                         const boost::python::list &pyvals) {
  std::vector<int64_t> vals = pythonListToCppLongIntVector(pyvals);
  return ctxt -= vals;
}

BGVCiphertext operator-=(BGVCiphertext &ctxt,
                         const boost::python::numpy::ndarray &pyvals) {
  std::vector<int64_t> vals = numpyListToCppLongIntVector(pyvals);
  return ctxt -= vals;
}

BGVCiphertext operator-(BGVCiphertext ctxt, const boost::python::list &vals) {
  return ctxt -= vals;
}

BGVCiphertext operator-(BGVCiphertext ctxt,
                        const boost::python::numpy::ndarray &vals) {
  return ctxt -= vals;
}

BGVCiphertext operator-(const BGVCiphertext &ctxt) {
  return ctxt.cipher->GetCryptoContext()->EvalNegate(ctxt.cipher);
}

BGVCiphertext operator-=(BGVCiphertext &ctxt, int64_t val) {
  ctxt += -val;
  return ctxt;
}

BGVCiphertext operator-(BGVCiphertext ctxt, int64_t val) { return ctxt -= val; }

BGVCiphertext operator-(int64_t val, const BGVCiphertext &ctxt) {
  auto ncipher = -ctxt;
  return ncipher += val;
}

BGVCiphertext operator-(const std::vector<int64_t> &vals, BGVCiphertext ctxt) {
  return ctxt -= vals;
}

BGVCiphertext operator-(const boost::python::list &vals,
                        const BGVCiphertext &ctxt) {
  auto ncipher = -ctxt;
  return ncipher += vals;
}

BGVCiphertext operator-(const boost::python::numpy::ndarray &vals,
                        const BGVCiphertext &ctxt) {
  auto ncipher = -ctxt;
  return ncipher -= vals;
}

// TODO: is this still needed? is it any different for BGV/int operations?
BGVCiphertext BGVMultiplySingletonIntAndAdd(const BGVCiphertext &ctxt,
                                            int64_t val) {
  if (val == 0) {
    return (ctxt - ctxt); // zero
  }
  if (val < 0) {
    return BGVMultiplySingletonIntAndAdd(-ctxt, -val);
  }

  BGVCiphertext doubles = ctxt;
  BGVCiphertext result = (ctxt - ctxt); // zero
  while (val > 0) {
    if (val & 1) {
      result += doubles;
    }
    doubles = doubles + doubles;
    val >>= 1;
  }
  return result;
}

BGVCiphertext operator*=(BGVCiphertext &ctxt, int64_t val) {
  ctxt = BGVMultiplySingletonIntAndAdd(ctxt, val);
  return ctxt;
}

BGVCiphertext operator*=(BGVCiphertext &ctxt, std::vector<int64_t> vals) {
  size_t N = ctxt.cipher->GetEncodingParameters()->GetBatchSize();
  if (vals.size() != N) {
    std::string s = fmt::format("Provided vector has length = {}, but the "
                                "CryptoContext batch size = {}",
                                vals.size(), N);
    throw std::runtime_error(s);
  }
  if (ctxt.getTowersRemaining() <= 2) {
    // TODO: verify if this condition is the same for BGV
    throw std::runtime_error(
        fmt::format("Insufficient number of towers remaining to perform a "
                    "multiplication = {}",
                    ctxt.getTowersRemaining()));
  }
  size_t final_size = ctxt.cipher->GetCryptoContext()->GetRingDimension() / 2;
  tileVector(vals, final_size);
  auto ptxt = ctxt.cipher->GetCryptoContext()->MakePackedPlaintext(vals);
  ctxt.cipher = ctxt.cipher->GetCryptoContext()->EvalMult(ctxt.cipher, ptxt);
  return ctxt;
}

BGVCiphertext operator*=(BGVCiphertext &ctxt,
                         const boost::python::list &pyvals) {
  std::vector<int64_t> vals = pythonListToCppLongIntVector(pyvals);
  return ctxt *= vals;
}

BGVCiphertext operator*=(BGVCiphertext &ctxt,
                         const boost::python::numpy::ndarray &pyvals) {
  std::vector<int64_t> vals = numpyListToCppLongIntVector(pyvals);
  return ctxt *= vals;
}

BGVCiphertext operator*(BGVCiphertext ctxt, int64_t val) { return ctxt *= val; }

BGVCiphertext operator*(int64_t val, BGVCiphertext ctxt) { return ctxt *= val; }

BGVCiphertext operator*(const std::vector<int64_t> &vals, BGVCiphertext ctxt) {
  return ctxt *= vals;
}

BGVCiphertext operator*(BGVCiphertext ctxt, const std::vector<int64_t> &vals) {
  return ctxt *= vals;
}

BGVCiphertext operator*(BGVCiphertext ctxt, const boost::python::list &vals) {
  return ctxt *= vals;
}

BGVCiphertext operator*(const boost::python::list &vals, BGVCiphertext ctxt) {
  return ctxt *= vals;
}

BGVCiphertext operator*(BGVCiphertext ctxt,
                        const boost::python::numpy::ndarray &vals) {
  return ctxt *= vals;
}

BGVCiphertext operator*(const boost::python::numpy::ndarray &vals,
                        BGVCiphertext ctxt) {
  return ctxt *= vals;
}

} // namespace pyOpenFHE_BGV
