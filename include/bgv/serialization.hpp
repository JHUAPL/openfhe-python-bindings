// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef BGV_SERIALIZATION_OPENFHE_PYTHON_BINDINGS_H
#define BGV_SERIALIZATION_OPENFHE_PYTHON_BINDINGS_H

#include <complex>
#include <stdexcept>
#include <vector>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "openfhe.h"
#include "utils/utils.hpp"

#include "bgv/BGV_ciphertext_extension.hpp"
#include "bgv/BGV_key_operations.hpp"

using namespace lbcrypto;

namespace pyOpenFHE_BGV {

// We turned the Serial::SerType into an enum
enum class SerType { BINARY, JSON };

PyObject *SerializeToBytes_EvalMultKey_CryptoContext(
    BGVCryptoContext &self, const pyOpenFHE_BGV::SerType sertype);
bool DeserializeFromBytes_EvalMultKey_CryptoContext(
    BGVCryptoContext &self, boost::python::object py_buffer,
    const pyOpenFHE_BGV::SerType sertype);
PyObject *SerializeToBytes_EvalAutomorphismKey_CryptoContext(
    BGVCryptoContext &self, const pyOpenFHE_BGV::SerType sertype);
bool DeserializeFromBytes_EvalAutomorphismKey_CryptoContext(
    BGVCryptoContext &self, boost::python::object py_buffer,
    const pyOpenFHE_BGV::SerType sertype);

PyObject *SerializeToBytes_Ciphertext(const pyOpenFHE_BGV::BGVCiphertext &obj,
                                      const pyOpenFHE_BGV::SerType sertype);
pyOpenFHE_BGV::BGVCiphertext
DeserializeFromBytes_Ciphertext(boost::python::object py_buffer,
                                const pyOpenFHE_BGV::SerType sertype);
PyObject *SerializeToBytes_PublicKey(const PublicKey<DCRTPoly> &obj,
                                     const pyOpenFHE_BGV::SerType sertype);
PublicKey<DCRTPoly>
DeserializeFromBytes_PublicKey(boost::python::object py_buffer,
                               const pyOpenFHE_BGV::SerType sertype);
PyObject *SerializeToBytes_PrivateKey(const PrivateKey<DCRTPoly> &obj,
                                      const pyOpenFHE_BGV::SerType sertype);
PrivateKey<DCRTPoly>
DeserializeFromBytes_PrivateKey(boost::python::object py_buffer,
                                const pyOpenFHE_BGV::SerType sertype);

bool SerializeToFile_CryptoContext(const std::string &filename,
                                   const BGVCryptoContext &obj,
                                   const pyOpenFHE_BGV::SerType sertype);
CryptoContext<DCRTPoly>
DeserializeFromFile_CryptoContext(const std::string &filename,
                                  const pyOpenFHE_BGV::SerType sertype);

bool SerializeToFile_Ciphertext(const std::string &filename,
                                const pyOpenFHE_BGV::BGVCiphertext &obj,
                                const pyOpenFHE_BGV::SerType sertype);
bool SerializeToFile_EvalMultKey_CryptoContext(
    BGVCryptoContext &self, const std::string &filename,
    const pyOpenFHE_BGV::SerType sertype);
bool SerializeToFile_EvalAutomorphismKey_CryptoContext(
    BGVCryptoContext &self, const std::string &filename,
    const pyOpenFHE_BGV::SerType sertype);
bool SerializeToFile_PublicKey(const std::string &filename,
                               const PublicKey<DCRTPoly> &obj,
                               const pyOpenFHE_BGV::SerType sertype);
bool SerializeToFile_PrivateKey(const std::string &filename,
                                const PrivateKey<DCRTPoly> &obj,
                                const pyOpenFHE_BGV::SerType sertype);

pyOpenFHE_BGV::BGVCiphertext
DeserializeFromFile_Ciphertext(const std::string &filename,
                               const pyOpenFHE_BGV::SerType sertype);
PublicKey<DCRTPoly>
DeserializeFromFile_PublicKey(const std::string &filename,
                              const pyOpenFHE_BGV::SerType sertype);
PrivateKey<DCRTPoly>
DeserializeFromFile_PrivateKey(const std::string &filename,
                               const pyOpenFHE_BGV::SerType sertype);
bool DeserializeFromFile_EvalMultKey_CryptoContext(
    BGVCryptoContext &self, const std::string &filename,
    const pyOpenFHE_BGV::SerType sertype);
bool DeserializeFromFile_EvalAutomorphismKey_CryptoContext(
    BGVCryptoContext &self, const std::string &filename,
    const pyOpenFHE_BGV::SerType sertype);

} // namespace pyOpenFHE_BGV

#endif /* BGV_SERIALIZATION_OPENFHE_PYTHON_BINDINGS_H */
