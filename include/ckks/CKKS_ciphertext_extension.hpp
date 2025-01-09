// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef CKKS_PALISADE_PYTHON_CIPHERTEXT_H
#define CKKS_PALISADE_PYTHON_CIPHERTEXT_H

#include <complex>
#include <vector>

#include <fmt/format.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "openfhe.h"

#include "ckks/CKKS_key_operations.hpp"
#include "utils/utils.hpp"

#include "constants.h"
#include "encoding/encodingparams.h"
#include "scheme/scheme-utils.h"
#include "utils/exceptions.hpp"

using namespace lbcrypto;

namespace pyOpenFHE_CKKS {

class CKKSCryptoContext;

class CKKSCiphertext {

public:
  Ciphertext<DCRTPoly> cipher;

  // empty constructor
  CKKSCiphertext(){};

  // wrapper constructor
  CKKSCiphertext(Ciphertext<DCRTPoly> cipher) : cipher(cipher){};

  // copy constructor
  CKKSCiphertext(const CKKSCiphertext &ctxt) {
    cipher = Ciphertext<DCRTPoly>(ctxt.cipher);
  };

  CKKSCiphertext array_ufunc(const boost::python::object ufunc,
                             const boost::python::str method,
                             const boost::python::numpy::ndarray &vals,
                             const CKKSCiphertext &cipher);

  uint64_t getPlaintextModulus(void) const {
    return cipher->GetCryptoContext()
        ->GetEncodingParams()
        ->GetPlaintextModulus();
  };

  CKKSCryptoContext getCryptoContext(void) const;

  double getScalingFactor(void) const {
    // TODO
    // const auto cryptoParams =
    // std::dynamic_pointer_cast<CryptoParametersRNS>(cipher->GetCryptoParameters());
    // double scFactor = cryptoParams->GetScalingFactorInt(cipher->GetLevel());
    // return scFactor;
    return cipher->GetScalingFactor();
  };

  uint64_t getBatchSize(void) const {
    return cipher->GetCryptoContext()->GetEncodingParams()->GetBatchSize();
  };

  /*
   * multiplicative depth performed & remaining.
   * GetLevel() return holds the number of rescalings performed
   * before getting this ciphertext - initially 0,
   * which is backwards for our purposes.
   */
  uint64_t getMultLevel(void) const { return cipher->GetLevel(); };

  uint64_t getTowersRemaining(void) const {
    const std::vector<DCRTPoly> &cv = cipher->GetElements();
    usint sizeQl = cv[0].GetNumOfElements();
    return sizeQl;
  }

  // TODO broken
  // uint64_t getMultsRemaining(void) const {
  //     CryptoContext<DCRTPoly> cc = cipher->GetCryptoContext();

  //     std::cout << "CKKS scheme is using ring dimension = " <<
  //     cc->GetRingDimension() << std::endl; std::cout << "batch size = " <<
  //     cc->GetEncodingParams()->GetBatchSize() << std::endl; std::cout <<
  //     "mult depth = " << cc->GetEncodingParams()->GetMultDepth() <<
  //     std::endl;

  //     // uint64_t mults_used = cipher->GetLevel();
  //     // return max_depth - mults_used;

        //     return 0;
        // }

        // an alternative to rescale?
        // lowers TowersRemaining to towersLeft
        // I don't really know what happens if towersLeft >= TowersRemaining so I'll just not do that
        CKKSCiphertext compress(size_t towersLeft) const {
            if (getTowersRemaining() <= towersLeft) {
                throw std::runtime_error(fmt::format("Cannot compress to {} towers, towers remaining = {}", towersLeft, getTowersRemaining()));
            }
            auto cipher2 = cipher->GetCryptoContext()->GetScheme()->Compress(cipher, towersLeft);
            return CKKSCiphertext(cipher2);
        }


  // TODO broken
  CKKSCiphertext Rescale(size_t levels = 1) const {
    if (getTowersRemaining() <= 1 + levels) {
      throw std::runtime_error(
          fmt::format("Insufficient number of towers remaining = {} to perform "
                      "{} rescalings",
                      getTowersRemaining(), levels));
    }
    auto algo = cipher->GetCryptoContext()->GetScheme();
    auto cipher2 = algo->ModReduce(cipher, levels);
    return CKKSCiphertext(cipher2);
  }
};

// a whole load of operators
// we need to specify ALL of these, and then specify them again in the
// bindings...

bool operator==(const CKKSCiphertext &c1, const CKKSCiphertext &c2);
bool operator!=(const CKKSCiphertext &c1, const CKKSCiphertext &c2);
CKKSCiphertext operator+=(CKKSCiphertext &c1, const CKKSCiphertext &c2);
CKKSCiphertext operator+(CKKSCiphertext c1, const CKKSCiphertext &c2);
CKKSCiphertext operator-=(CKKSCiphertext &c1, const CKKSCiphertext &c2);
CKKSCiphertext operator-(CKKSCiphertext c1, const CKKSCiphertext &c2);
CKKSCiphertext operator*=(CKKSCiphertext &c1, const CKKSCiphertext &c2);
CKKSCiphertext operator*(CKKSCiphertext c1, const CKKSCiphertext &c2);
CKKSCiphertext operator<<=(CKKSCiphertext &ctxt, int r);
CKKSCiphertext CKKSRotateEvalAtIndex(CKKSCiphertext ctxt, int r);
boost::python::list CKKSHoistedRotations(const CKKSCiphertext &ctxt,
                                         const boost::python::list &pylist);
CKKSCiphertext CKKSMultiplySingletonDirect(CKKSCiphertext ctxt, double val);
CKKSCiphertext CKKSMultiplySingletonIntDoubleAndAdd(const CKKSCiphertext &ctxt,
                                                    long int val);
CKKSCiphertext operator>>=(CKKSCiphertext &ctxt, double r);
CKKSCiphertext operator<<(CKKSCiphertext ctxt, double r);
CKKSCiphertext operator>>(CKKSCiphertext ctxt, double r);
CKKSCiphertext operator+=(CKKSCiphertext &ctxt, double val);
CKKSCiphertext operator+(CKKSCiphertext ctxt, double val);
CKKSCiphertext operator+(double val, CKKSCiphertext ctxt);
CKKSCiphertext operator+=(CKKSCiphertext &ctxt, std::vector<double> vals);
CKKSCiphertext operator+=(CKKSCiphertext &ctxt,
                          const boost::python::list &pyvals);
CKKSCiphertext operator+=(CKKSCiphertext &ctxt,
                          const boost::python::numpy::ndarray &pyvals);
CKKSCiphertext operator+(CKKSCiphertext ctxt, const std::vector<double> &vals);
CKKSCiphertext operator+(const std::vector<double> &vals, CKKSCiphertext ctxt);
CKKSCiphertext operator+(CKKSCiphertext ctxt, const boost::python::list &vals);
CKKSCiphertext operator+(CKKSCiphertext ctxt,
                         const boost::python::numpy::ndarray &vals);
CKKSCiphertext operator+(const boost::python::list &vals, CKKSCiphertext ctxt);
CKKSCiphertext operator+(const boost::python::numpy::ndarray &vals,
                         CKKSCiphertext ctxt);
CKKSCiphertext operator-=(CKKSCiphertext &ctxt, std::vector<double> vals);
CKKSCiphertext operator-=(CKKSCiphertext &ctxt,
                          const boost::python::list &pyvals);
CKKSCiphertext operator-=(CKKSCiphertext &ctxt,
                          const boost::python::numpy::ndarray &pyvals);
CKKSCiphertext operator-(CKKSCiphertext ctxt, const boost::python::list &vals);
CKKSCiphertext operator-(CKKSCiphertext ctxt,
                         const boost::python::numpy::ndarray &vals);
CKKSCiphertext operator-(const CKKSCiphertext &ctxt);
CKKSCiphertext operator-=(CKKSCiphertext &ctxt, double val);
CKKSCiphertext operator-(CKKSCiphertext ctxt, double val);
CKKSCiphertext operator-(double val, const CKKSCiphertext &ctxt);
CKKSCiphertext operator-(CKKSCiphertext ctxt, const std::vector<double> &vals);
CKKSCiphertext operator-(const std::vector<double> &vals, CKKSCiphertext ctxt);
CKKSCiphertext operator-(const boost::python::list &vals,
                         const CKKSCiphertext &ctxt);
CKKSCiphertext operator-(const boost::python::numpy::ndarray &vals,
                         const CKKSCiphertext &ctxt);
CKKSCiphertext operator*=(CKKSCiphertext &ctxt, std::vector<double> vals);
CKKSCiphertext operator*=(CKKSCiphertext &ctxt, double val);
CKKSCiphertext operator*=(CKKSCiphertext &ctxt, long int val);
CKKSCiphertext operator*=(CKKSCiphertext &ctxt,
                          const boost::python::list &pyvals);
CKKSCiphertext operator*=(CKKSCiphertext &ctxt,
                          const boost::python::numpy::ndarray &pyvals);
CKKSCiphertext operator*(CKKSCiphertext ctxt, double val);
CKKSCiphertext operator*(double val, CKKSCiphertext ctxt);
CKKSCiphertext operator*(CKKSCiphertext ctxt, long int val);
CKKSCiphertext operator*(long int val, CKKSCiphertext ctxt);
CKKSCiphertext operator*(CKKSCiphertext ctxt, const std::vector<double> &vals);
CKKSCiphertext operator*(const std::vector<double> &vals, CKKSCiphertext ctxt);
CKKSCiphertext operator*(CKKSCiphertext ctxt, const boost::python::list &vals);
CKKSCiphertext operator*(const boost::python::list &vals, CKKSCiphertext ctxt);
CKKSCiphertext operator*(CKKSCiphertext ctxt,
                         const boost::python::numpy::ndarray &vals);
CKKSCiphertext operator*(const boost::python::numpy::ndarray &vals,
                         CKKSCiphertext ctxt);

} // namespace pyOpenFHE_CKKS

#endif /* CKKS_PALISADE_PYTHON_CIPHERTEXT_H */