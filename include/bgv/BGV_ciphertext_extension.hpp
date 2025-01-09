// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef BGV_OPENFHE_PYTHON_CIPHERTEXT_H
#define BGV_OPENFHE_PYTHON_CIPHERTEXT_H

#include <complex>
#include <string>
#include <vector>

#include <fmt/format.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "openfhe.h"

#include "bgv/BGV_key_operations.hpp"
#include "utils/exceptions.hpp"
#include "utils/utils.hpp"

#include "constants.h"
#include "encoding/encodingparams.h"
#include "scheme/scheme-utils.h"
#include "utils/exceptions.hpp"

using namespace lbcrypto;

namespace pyOpenFHE_BGV {

class BGVCryptoContext; // forward declaration so we can declare
                        // getCryptoContext()

class BGVCiphertext {

public:
  // I kind of want to make this private and hide behind a getter but maybe
  // that's just Java brain
  Ciphertext<DCRTPoly> cipher;

  // empty constructor
  BGVCiphertext(){};

  // wrapper constructor
  BGVCiphertext(Ciphertext<DCRTPoly> cipher) : cipher(cipher){};

  // copy constructor
  BGVCiphertext(const BGVCiphertext &ctxt) {
    cipher = Ciphertext<DCRTPoly>(ctxt.cipher);
  };

  BGVCiphertext array_ufunc(const boost::python::object ufunc,
                            const boost::python::str method,
                            const boost::python::numpy::ndarray &vals,
                            const BGVCiphertext &cipher);

  uint64_t getPlaintextModulus(void) const {
    return cipher->GetCryptoContext()
        ->GetEncodingParams()
        ->GetPlaintextModulus();
  };

  BGVCryptoContext getCryptoContext(void) const;

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

  //     std::cout << "BGV scheme is using ring dimension = " <<
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
  // I don't really know what happens if towersLeft >= TowersRemaining so I'll
  // just not do that
  BGVCiphertext compress(size_t towersLeft) const {
    if (getTowersRemaining() <= towersLeft) {
      throw std::runtime_error(
          fmt::format("Cannot compress to {} towers, towers remaining = {}",
                      towersLeft, getTowersRemaining()));
    }
    auto cipher2 =
        cipher->GetCryptoContext()->GetScheme()->Compress(cipher, towersLeft);
    return BGVCiphertext(cipher2);
  }

  // TODO broken
  BGVCiphertext rescale(size_t levels = 1) const {
    if (getTowersRemaining() <= 1 + levels) {
      throw std::runtime_error(
          fmt::format("Insufficient number of towers remaining = {} to perform "
                      "{} rescalings",
                      getTowersRemaining(), levels));
    }
    // ModReduce will only work if ScalingTechnique is FIXEDMANUAL
    // default technique is FLEXIBLEAUTOEXT, which is useful
    // so this won't work
    // auto algo = cipher->GetCryptoContext()->GetScheme();
    // auto cipher2 = algo->ModReduce(cipher, levels);
    // compression *may* do what we want?
    // but it has to be inverted
    auto cipher2 = compress(getTowersRemaining() - levels);
    return BGVCiphertext(cipher2);
  }
};

// a whole load of operators
// we need to specify ALL of these, and then specify them again in the
// bindings...

bool operator==(const BGVCiphertext &c1, const BGVCiphertext &c2);
bool operator!=(const BGVCiphertext &c1, const BGVCiphertext &c2);
BGVCiphertext operator+=(BGVCiphertext &c1, const BGVCiphertext &c2);
BGVCiphertext operator+(BGVCiphertext c1, const BGVCiphertext &c2);
BGVCiphertext operator-=(BGVCiphertext &c1, const BGVCiphertext &c2);
BGVCiphertext operator-(BGVCiphertext c1, const BGVCiphertext &c2);
BGVCiphertext operator*=(BGVCiphertext &c1, const BGVCiphertext &c2);
BGVCiphertext operator*(BGVCiphertext c1, const BGVCiphertext &c2);
BGVCiphertext operator<<=(BGVCiphertext &ctxt, int r);
BGVCiphertext BGVRotateEvalAtIndex(BGVCiphertext ctxt, int r);
boost::python::list BGVHoistedRotations(const BGVCiphertext &ctxt,
                                        const boost::python::list &pylist);
BGVCiphertext BGVMultiplySingletonDirect(BGVCiphertext ctxt, int64_t val);
BGVCiphertext BGVMultiplySingletonIntAndAdd(const BGVCiphertext &ctxt,
                                            int64_t val);
BGVCiphertext operator>>=(BGVCiphertext &ctxt, int r);
BGVCiphertext operator<<(BGVCiphertext ctxt, int r);
BGVCiphertext operator>>(BGVCiphertext ctxt, int r);
BGVCiphertext operator+=(BGVCiphertext &ctxt, int64_t val);
BGVCiphertext operator+(BGVCiphertext ctxt, int64_t val);
BGVCiphertext operator+(int64_t val, BGVCiphertext ctxt);
BGVCiphertext operator+=(BGVCiphertext &ctxt, std::vector<int64_t> vals);
BGVCiphertext operator+=(BGVCiphertext &ctxt,
                         const boost::python::list &pyvals);
BGVCiphertext operator+=(BGVCiphertext &ctxt,
                         const boost::python::numpy::ndarray &pyvals);
BGVCiphertext operator+(BGVCiphertext ctxt, const std::vector<int64_t> &vals);
BGVCiphertext operator+(const std::vector<int64_t> &vals, BGVCiphertext ctxt);
BGVCiphertext operator+(BGVCiphertext ctxt, const boost::python::list &vals);
BGVCiphertext operator+(BGVCiphertext ctxt,
                        const boost::python::numpy::ndarray &vals);
BGVCiphertext operator+(const boost::python::list &vals, BGVCiphertext ctxt);
BGVCiphertext operator+(const boost::python::numpy::ndarray &vals,
                        BGVCiphertext ctxt);
BGVCiphertext operator-=(BGVCiphertext &ctxt, std::vector<int64_t> vals);
BGVCiphertext operator-=(BGVCiphertext &ctxt,
                         const boost::python::list &pyvals);
BGVCiphertext operator-=(BGVCiphertext &ctxt,
                         const boost::python::numpy::ndarray &pyvals);
BGVCiphertext operator-(BGVCiphertext ctxt, const boost::python::list &vals);
BGVCiphertext operator-(BGVCiphertext ctxt,
                        const boost::python::numpy::ndarray &vals);
BGVCiphertext operator-(const BGVCiphertext &ctxt);
BGVCiphertext operator-=(BGVCiphertext &ctxt, int64_t val);
BGVCiphertext operator-(BGVCiphertext ctxt, int64_t val);
BGVCiphertext operator-(int64_t val, const BGVCiphertext &ctxt);
BGVCiphertext operator-(BGVCiphertext ctxt, const std::vector<int64_t> &vals);
BGVCiphertext operator-(const std::vector<int64_t> &vals, BGVCiphertext ctxt);
BGVCiphertext operator-(const boost::python::list &vals,
                        const BGVCiphertext &ctxt);
BGVCiphertext operator-(const boost::python::numpy::ndarray &vals,
                        const BGVCiphertext &ctxt);
BGVCiphertext operator*=(BGVCiphertext &ctxt, std::vector<int64_t> vals);
BGVCiphertext operator*=(BGVCiphertext &ctxt, int64_t val);
BGVCiphertext operator*=(BGVCiphertext &ctxt, int64_t val);
BGVCiphertext operator*=(BGVCiphertext &ctxt,
                         const boost::python::list &pyvals);
BGVCiphertext operator*=(BGVCiphertext &ctxt,
                         const boost::python::numpy::ndarray &pyvals);
BGVCiphertext operator*(BGVCiphertext ctxt, int64_t val);
BGVCiphertext operator*(int64_t val, BGVCiphertext ctxt);
BGVCiphertext operator*(BGVCiphertext ctxt, int64_t val);
BGVCiphertext operator*(int64_t val, BGVCiphertext ctxt);
BGVCiphertext operator*(BGVCiphertext ctxt, const std::vector<int64_t> &vals);
BGVCiphertext operator*(const std::vector<int64_t> &vals, BGVCiphertext ctxt);
BGVCiphertext operator*(BGVCiphertext ctxt, const boost::python::list &vals);
BGVCiphertext operator*(const boost::python::list &vals, BGVCiphertext ctxt);
BGVCiphertext operator*(BGVCiphertext ctxt,
                        const boost::python::numpy::ndarray &vals);
BGVCiphertext operator*(const boost::python::numpy::ndarray &vals,
                        BGVCiphertext ctxt);

} // namespace pyOpenFHE_BGV

#endif /* BGV_OPENFHE_PYTHON_CIPHERTEXT_H */
