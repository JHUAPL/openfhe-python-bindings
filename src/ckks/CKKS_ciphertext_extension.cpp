// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#include "openfhe.h"

#include <complex>
#include <string>
#include <vector>

// string formatting for exceptions
#include <fmt/format.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "ckks/CKKS_ciphertext_extension.hpp"
#include "ckks/CKKS_key_operations.hpp"
#include "utils/exceptions.hpp"
#include "utils/rotate_utils.hpp"
#include "utils/utils.hpp"

using namespace lbcrypto;
using namespace boost::python;
using namespace boost::python::numpy;
using namespace pyOpenFHE;

namespace pyOpenFHE_CKKS {

CKKSCryptoContext CKKSCiphertext::getCryptoContext(void) const {
  return CKKSCryptoContext(cipher->GetCryptoContext());
}

CKKSCiphertext CKKSCiphertext::array_ufunc(
    const boost::python::object ufunc, const boost::python::str method,
    const boost::python::numpy::ndarray &vals, const CKKSCiphertext &cipher) {
  // because this is defined, numpy will call into this instead of ufunc
  std::string op =
      boost::python::extract<std::string>(ufunc.attr("__name__"))();

  if (op == "multiply") {
    return cipher * vals;
  } else if (op == "add") {
    return cipher + vals;
  } else {
    throw pyOpenFHE::not_implemented_exception(
        fmt::format("operator {} between ndarray and CKKSCiphertext", op));
  }
}

// a whole load of operators
// unfortunately C++'s operator lookup isn't smart enough to infer double +
// CKKSCiphertext from CKKSCiphertext + double or I guess it's more that it
// doesn't assume commutativity either way, we need to specify ALL of these, and
// then specify them again in the bindings...

bool operator==(const CKKSCiphertext &c1, const CKKSCiphertext &c2) {
  return c1.cipher == c2.cipher;
}

bool operator!=(const CKKSCiphertext &c1, const CKKSCiphertext &c2) {
  return !(c1 == c2);
}

CKKSCiphertext operator+=(CKKSCiphertext &c1, const CKKSCiphertext &c2) {
  c1.cipher += c2.cipher;
  return c1;
}

CKKSCiphertext operator+(CKKSCiphertext c1, const CKKSCiphertext &c2) {
  return c1 += c2;
}

CKKSCiphertext operator-=(CKKSCiphertext &c1, const CKKSCiphertext &c2) {
  c1.cipher -= c2.cipher;
  return c1;
}

CKKSCiphertext operator-(CKKSCiphertext c1, const CKKSCiphertext &c2) {
  return c1 -= c2;
}

CKKSCiphertext operator*=(CKKSCiphertext &c1, const CKKSCiphertext &c2) {
  if ((c1.getTowersRemaining() <= 2) || (c2.getTowersRemaining() <= 2)) {
    throw std::runtime_error(
        fmt::format("Insufficient number of towers remaining to perform a "
                    "multiplication = {}, {}",
                    c1.getTowersRemaining(), c2.getTowersRemaining()));
  }
  c1.cipher *= c2.cipher;
  return c1;
}

CKKSCiphertext operator*(CKKSCiphertext c1, const CKKSCiphertext &c2) {
  return c1 *= c2;
}

/*
uses the positive and negative power-of-2 decomposition
e.g. decompose(15) = {16, -1}
*/
CKKSCiphertext EvalRotatePositiveNegativePow2(CKKSCiphertext &ctxt, int r) {
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
CKKSCiphertext EvalRotatePositivePow2(CKKSCiphertext &ctxt, int r) {
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

CKKSCiphertext CKKSRotateEvalAtIndex(CKKSCiphertext ctxt, int r) {
  ctxt.cipher = ctxt.cipher->GetCryptoContext()->EvalAtIndex(ctxt.cipher, r);
  return ctxt;
}

CKKSCiphertext operator<<=(CKKSCiphertext &ctxt, int r) {
  return EvalRotatePositiveNegativePow2(ctxt, r);
}

/*
TODO
need error checking to see if that rotation key actually exists.
Also this function should support integer numpy arrays as well
*/
boost::python::list CKKSHoistedRotations(const CKKSCiphertext &ctxt,
                                         const boost::python::list &pylist) {
  std::vector<int32_t> rotations = pyOpenFHE::pythonListToCppIntVector(pylist);

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
    result[i] = pyOpenFHE_CKKS::CKKSCiphertext(
        cc->EvalFastRotation(ctxt.cipher, rotations[i], M, cPrecomp));
  }
  return result;
}

CKKSCiphertext operator>>=(CKKSCiphertext &ctxt, double r) {
  return ctxt <<= (-r);
}

CKKSCiphertext operator<<(CKKSCiphertext ctxt, double r) { return ctxt <<= r; }

CKKSCiphertext operator>>(CKKSCiphertext ctxt, double r) { return ctxt >>= r; }

CKKSCiphertext operator+=(CKKSCiphertext &ctxt, double val) {
  std::vector<double> vals = {val};
  size_t dn = ctxt.cipher->GetEncodingParameters()->GetBatchSize();
  tileVector(vals, dn);
  auto ptxt = ctxt.cipher->GetCryptoContext()->MakeCKKSPackedPlaintext(vals);
  ctxt.cipher = ctxt.cipher->GetCryptoContext()->EvalAdd(ctxt.cipher, ptxt);
  return ctxt;
}

CKKSCiphertext operator+(CKKSCiphertext ctxt, double val) {
  return ctxt += val;
}

CKKSCiphertext operator+(double val, CKKSCiphertext ctxt) {
  return ctxt += val;
}

CKKSCiphertext operator+=(CKKSCiphertext &ctxt, std::vector<double> vals) {
  size_t N = ctxt.cipher->GetEncodingParameters()->GetBatchSize();
  if (vals.size() != N) {
    std::string s = fmt::format("Provided vector has length = {}, but the "
                                "CryptoContext batch size = {}",
                                vals.size(), N);
    throw std::runtime_error(s);
  }
  // size_t final_size = ctxt.cipher->GetCryptoContext()->GetRingDimension() /
  // 2; tileVector(vals, final_size);
  auto ptxt = ctxt.cipher->GetCryptoContext()->MakeCKKSPackedPlaintext(vals);
  ctxt.cipher = ctxt.cipher->GetCryptoContext()->EvalAdd(ctxt.cipher, ptxt);
  return ctxt;
}

CKKSCiphertext operator+=(CKKSCiphertext &ctxt,
                          const boost::python::list &pyvals) {
  std::vector<double> vals = pythonListToCppDoubleVector(pyvals);
  return ctxt += vals;
}

CKKSCiphertext operator+=(CKKSCiphertext &ctxt,
                          const boost::python::numpy::ndarray &pyvals) {
  std::vector<double> vals = numpyListToCppDoubleVector(pyvals);
  return ctxt += vals;
}

CKKSCiphertext operator+(CKKSCiphertext ctxt, const std::vector<double> &vals) {
  return ctxt += vals;
}

CKKSCiphertext operator+(CKKSCiphertext ctxt, const boost::python::list &vals) {
  return ctxt += vals;
}

CKKSCiphertext operator+(CKKSCiphertext ctxt,
                         const boost::python::numpy::ndarray &vals) {
  return ctxt += vals;
}

CKKSCiphertext operator+(const std::vector<double> &vals, CKKSCiphertext ctxt) {
  return ctxt += vals;
}

CKKSCiphertext operator+(const boost::python::list &vals, CKKSCiphertext ctxt) {
  return ctxt += vals;
}

CKKSCiphertext operator+(const boost::python::numpy::ndarray &vals,
                         CKKSCiphertext ctxt) {
  return ctxt += vals;
}

CKKSCiphertext operator-=(CKKSCiphertext &ctxt, std::vector<double> vals) {
  size_t N = ctxt.cipher->GetEncodingParameters()->GetBatchSize();
  if (vals.size() != N) {
    std::string s = fmt::format("Provided vector has length = {}, but the "
                                "CryptoContext batch size = {}",
                                vals.size(), N);
    throw std::runtime_error(s);
  }
  // size_t final_size = ctxt.cipher->GetCryptoContext()->GetRingDimension() /
  // 2; tileVector(vals, final_size);
  auto ptxt = ctxt.cipher->GetCryptoContext()->MakeCKKSPackedPlaintext(vals);
  ctxt.cipher = ctxt.cipher->GetCryptoContext()->EvalSub(ctxt.cipher, ptxt);
  return ctxt;
}

CKKSCiphertext operator-=(CKKSCiphertext &ctxt,
                          const boost::python::list &pyvals) {
  std::vector<double> vals = pythonListToCppDoubleVector(pyvals);
  return ctxt -= vals;
}

CKKSCiphertext operator-=(CKKSCiphertext &ctxt,
                          const boost::python::numpy::ndarray &pyvals) {
  std::vector<double> vals = numpyListToCppDoubleVector(pyvals);
  return ctxt -= vals;
}

CKKSCiphertext operator-(CKKSCiphertext ctxt, const boost::python::list &vals) {
  return ctxt -= vals;
}

CKKSCiphertext operator-(CKKSCiphertext ctxt,
                         const boost::python::numpy::ndarray &vals) {
  return ctxt -= vals;
}

CKKSCiphertext operator-(const CKKSCiphertext &ctxt) {
  return ctxt.cipher->GetCryptoContext()->EvalNegate(ctxt.cipher);
}

CKKSCiphertext operator-=(CKKSCiphertext &ctxt, double val) {
  ctxt += -val;
  return ctxt;
}

CKKSCiphertext operator-(CKKSCiphertext ctxt, double val) {
  return ctxt -= val;
}

CKKSCiphertext operator-(double val, const CKKSCiphertext &ctxt) {
  auto ncipher = -ctxt;
  return ncipher += val;
}

CKKSCiphertext operator-(const std::vector<double> &vals, CKKSCiphertext ctxt) {
  return ctxt -= vals;
}

CKKSCiphertext operator-(const boost::python::list &vals,
                         const CKKSCiphertext &ctxt) {
  auto ncipher = -ctxt;
  return ncipher += vals;
}

CKKSCiphertext operator-(const boost::python::numpy::ndarray &vals,
                         const CKKSCiphertext &ctxt) {
  auto ncipher = -ctxt;
  return ncipher -= vals;
}

CKKSCiphertext CKKSMultiplySingletonIntDoubleAndAdd(const CKKSCiphertext &ctxt,
                                                    long int val) {
  if (val == 0) {
    return (ctxt - ctxt); // zero
  }
  if (val < 0) {
    return CKKSMultiplySingletonIntDoubleAndAdd(-ctxt, -val);
  }

  CKKSCiphertext doubles = ctxt;
  CKKSCiphertext result = (ctxt - ctxt); // zero
  while (val > 0) {
    if (val & 1) {
      result += doubles;
    }
    doubles = doubles + doubles;
    val >>= 1;
  }
  return result;
}

// pass-by-value, so it makes a copy of ctxt
CKKSCiphertext CKKSMultiplySingletonDirect(CKKSCiphertext ctxt, double val) {
  if (ctxt.getTowersRemaining() <= 2) {
    throw std::runtime_error(
        fmt::format("Insufficient number of towers remaining to perform a "
                    "multiplication = {}",
                    ctxt.getTowersRemaining()));
  }
  std::vector<double> vals = {val};
  size_t dn = ctxt.cipher->GetEncodingParameters()->GetBatchSize();
  tileVector(vals, dn);
  auto ptxt = ctxt.cipher->GetCryptoContext()->MakeCKKSPackedPlaintext(vals);
  ctxt.cipher = ctxt.cipher->GetCryptoContext()->EvalMult(ctxt.cipher, ptxt);
  return ctxt;
}

CKKSCiphertext operator*=(CKKSCiphertext &ctxt, long int val) {
  if (abs(val) <= 256) {
    ctxt = CKKSMultiplySingletonIntDoubleAndAdd(ctxt, val);
  } else {
    ctxt = CKKSMultiplySingletonDirect(ctxt, (double)val);
  }
  return ctxt;
}

CKKSCiphertext operator*=(CKKSCiphertext &ctxt, double val) {
  if (ctxt.getTowersRemaining() <= 2) {
    throw std::runtime_error(
        fmt::format("Insufficient number of towers remaining to perform a "
                    "multiplication = {}",
                    ctxt.getTowersRemaining()));
  }
  ctxt = CKKSMultiplySingletonDirect(ctxt, val);
  return ctxt;
}

CKKSCiphertext operator*=(CKKSCiphertext &ctxt, std::vector<double> vals) {
  size_t N = ctxt.cipher->GetEncodingParameters()->GetBatchSize();
  if (vals.size() != N) {
    std::string s = fmt::format("Provided vector has length = {}, but the "
                                "CryptoContext batch size = {}",
                                vals.size(), N);
    throw std::runtime_error(s);
  }
  if (ctxt.getTowersRemaining() <= 2) {
    throw std::runtime_error(
        fmt::format("Insufficient number of towers remaining to perform a "
                    "multiplication = {}",
                    ctxt.getTowersRemaining()));
  }
  // size_t final_size = ctxt.cipher->GetCryptoContext()->GetRingDimension() /
  // 2; tileVector(vals, final_size);
  auto ptxt = ctxt.cipher->GetCryptoContext()->MakeCKKSPackedPlaintext(vals);
  ctxt.cipher = ctxt.cipher->GetCryptoContext()->EvalMult(ctxt.cipher, ptxt);
  return ctxt;
}

CKKSCiphertext operator*=(CKKSCiphertext &ctxt,
                          const boost::python::list &pyvals) {
  std::vector<double> vals = pythonListToCppDoubleVector(pyvals);
  return ctxt *= vals;
}

CKKSCiphertext operator*=(CKKSCiphertext &ctxt,
                          const boost::python::numpy::ndarray &pyvals) {
  std::vector<double> vals = numpyListToCppDoubleVector(pyvals);
  return ctxt *= vals;
}

CKKSCiphertext operator*(CKKSCiphertext ctxt, double val) {
  return ctxt *= val;
}

CKKSCiphertext operator*(double val, CKKSCiphertext ctxt) {
  return ctxt *= val;
}

CKKSCiphertext operator*(CKKSCiphertext ctxt, long int val) {
  return ctxt *= val;
}

CKKSCiphertext operator*(long int val, CKKSCiphertext ctxt) {
  return ctxt *= val;
}

CKKSCiphertext operator*(const std::vector<double> &vals, CKKSCiphertext ctxt) {
  return ctxt *= vals;
}

CKKSCiphertext operator*(CKKSCiphertext ctxt, const std::vector<double> &vals) {
  return ctxt *= vals;
}

CKKSCiphertext operator*(CKKSCiphertext ctxt, const boost::python::list &vals) {
  return ctxt *= vals;
}

CKKSCiphertext operator*(const boost::python::list &vals, CKKSCiphertext ctxt) {
  return ctxt *= vals;
}

CKKSCiphertext operator*(CKKSCiphertext ctxt,
                         const boost::python::numpy::ndarray &vals) {
  return ctxt *= vals;
}

CKKSCiphertext operator*(const boost::python::numpy::ndarray &vals,
                         CKKSCiphertext ctxt) {
  return ctxt *= vals;
}

} // namespace pyOpenFHE_CKKS