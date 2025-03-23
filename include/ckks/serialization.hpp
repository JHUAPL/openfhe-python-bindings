// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef OPENFHE_PYTHON_SERIALIZATION_H
#define OPENFHE_PYTHON_SERIALIZATION_H

#include <complex>
#include <stdexcept>
#include <vector>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "openfhe.h"
#include "utils/utils.hpp"

#include "ckks/CKKS_ciphertext_extension.hpp"
#include "ckks/CKKS_key_operations.hpp"

using namespace lbcrypto;

namespace pyOpenFHE_CKKS {

// We turned the Serial::SerType into an enum
enum class SerType { BINARY, JSON };

PyObject *SerializeToBytes_EvalMultKey_CryptoContext(
    CKKSCryptoContext &self, const pyOpenFHE_CKKS::SerType sertype);
bool DeserializeFromBytes_EvalMultKey_CryptoContext(
    CKKSCryptoContext &self, boost::python::object py_buffer,
    const pyOpenFHE_CKKS::SerType sertype);
PyObject *SerializeToBytes_EvalAutomorphismKey_CryptoContext(
    CKKSCryptoContext &self, const pyOpenFHE_CKKS::SerType sertype);
bool DeserializeFromBytes_EvalAutomorphismKey_CryptoContext(
    CKKSCryptoContext &self, boost::python::object py_buffer,
    const pyOpenFHE_CKKS::SerType sertype);

PyObject *SerializeToBytes_Ciphertext(const pyOpenFHE_CKKS::CKKSCiphertext &obj,
                                      const pyOpenFHE_CKKS::SerType sertype);
pyOpenFHE_CKKS::CKKSCiphertext
DeserializeFromBytes_Ciphertext(boost::python::object py_buffer,
                                const pyOpenFHE_CKKS::SerType sertype);
PyObject *SerializeToBytes_PublicKey(const PublicKey<DCRTPoly> &obj,
                                     const pyOpenFHE_CKKS::SerType sertype);
PublicKey<DCRTPoly>
DeserializeFromBytes_PublicKey(boost::python::object py_buffer,
                               const pyOpenFHE_CKKS::SerType sertype);
PyObject *SerializeToBytes_PrivateKey(const PrivateKey<DCRTPoly> &obj,
                                      const pyOpenFHE_CKKS::SerType sertype);
PrivateKey<DCRTPoly>
DeserializeFromBytes_PrivateKey(boost::python::object py_buffer,
                                const pyOpenFHE_CKKS::SerType sertype);

bool SerializeToFile_CryptoContext(const std::string &filename,
                                   const CKKSCryptoContext &obj,
                                   const pyOpenFHE_CKKS::SerType sertype);
CryptoContext<DCRTPoly>
DeserializeFromFile_CryptoContext(const std::string &filename,
                                  const pyOpenFHE_CKKS::SerType sertype);

bool SerializeToFile_Ciphertext(const std::string &filename,
                                const pyOpenFHE_CKKS::CKKSCiphertext &obj,
                                const pyOpenFHE_CKKS::SerType sertype);
bool SerializeToFile_EvalMultKey_CryptoContext(
    CKKSCryptoContext &self, const std::string &filename,
    const pyOpenFHE_CKKS::SerType sertype);
bool SerializeToFile_EvalAutomorphismKey_CryptoContext(
    CKKSCryptoContext &self, const std::string &filename,
    const pyOpenFHE_CKKS::SerType sertype);
bool SerializeToFile_PublicKey(const std::string &filename,
                               const PublicKey<DCRTPoly> &obj,
                               const pyOpenFHE_CKKS::SerType sertype);
bool SerializeToFile_PrivateKey(const std::string &filename,
                                const PrivateKey<DCRTPoly> &obj,
                                const pyOpenFHE_CKKS::SerType sertype);

pyOpenFHE_CKKS::CKKSCiphertext
DeserializeFromFile_Ciphertext(const std::string &filename,
                               const pyOpenFHE_CKKS::SerType sertype);
PublicKey<DCRTPoly>
DeserializeFromFile_PublicKey(const std::string &filename,
                              const pyOpenFHE_CKKS::SerType sertype);
PrivateKey<DCRTPoly>
DeserializeFromFile_PrivateKey(const std::string &filename,
                               const pyOpenFHE_CKKS::SerType sertype);
bool DeserializeFromFile_EvalMultKey_CryptoContext(
    CKKSCryptoContext &self, const std::string &filename,
    const pyOpenFHE_CKKS::SerType sertype);
bool DeserializeFromFile_EvalAutomorphismKey_CryptoContext(
    CKKSCryptoContext &self, const std::string &filename,
    const pyOpenFHE_CKKS::SerType sertype);

} // namespace pyOpenFHE_CKKS

#endif /* OPENFHE_PYTHON_SERIALIZATION_H */