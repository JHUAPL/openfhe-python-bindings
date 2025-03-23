// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

// bindings for the serialization functions

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "openfhe.h"

#include "ckks/serialization.hpp"

namespace pyOpenFHE_CKKS {

void export_CKKS_serialization_boost() {

  enum_<pyOpenFHE_CKKS::SerType>("SerType")
      .value("BINARY", pyOpenFHE_CKKS::SerType::BINARY)
      .value("JSON", pyOpenFHE_CKKS::SerType::JSON);

  /*
  TODO: is there a better way to handle the Deserialize operation?
  */
  def("SerializeToBytes", SerializeToBytes_Ciphertext);
  def("SerializeToBytes", SerializeToBytes_PublicKey);
  def("SerializeToBytes", SerializeToBytes_PrivateKey);

  def("SerializeToFile", SerializeToFile_Ciphertext);
  def("SerializeToFile", SerializeToFile_PublicKey);
  def("SerializeToFile", SerializeToFile_PrivateKey);

  def("DeserializeFromBytes_Ciphertext", DeserializeFromBytes_Ciphertext);
  def("DeserializeFromBytes_PublicKey", DeserializeFromBytes_PublicKey);
  def("DeserializeFromBytes_PrivateKey", DeserializeFromBytes_PrivateKey);

  def("DeserializeFromFile_Ciphertext", DeserializeFromFile_Ciphertext);
  def("DeserializeFromFile_PublicKey", DeserializeFromFile_PublicKey);
  def("DeserializeFromFile_PrivateKey", DeserializeFromFile_PrivateKey);

  /* TODO: disabled */
  def("SerializeToFile_EvalMultKey_CryptoContext",
      &SerializeToFile_EvalMultKey_CryptoContext);
  def("DeserializeFromFile_CryptoContext", &DeserializeFromFile_CryptoContext);

  /*
  The difference is naming between these and the above functions is unfortunate,
  but hard to avoid given how OpenFHE works in C++.
  Deserializing these keys requires being passed a CryptoContext object,
  while the above keys do not.
  */
  def("SerializeToFile_EvalMultKey_CryptoContext",
      &SerializeToFile_EvalMultKey_CryptoContext);
  def("SerializeToFile_EvalAutomorphismKey_CryptoContext",
      &SerializeToFile_EvalAutomorphismKey_CryptoContext);

  def("DeserializeFromFile_EvalMultKey_CryptoContext",
      &DeserializeFromFile_EvalMultKey_CryptoContext);
  def("DeserializeFromFile_EvalAutomorphismKey_CryptoContext",
      &DeserializeFromFile_EvalAutomorphismKey_CryptoContext);

  def("SerializeToBytes_EvalMultKey_CryptoContext",
      &SerializeToBytes_EvalMultKey_CryptoContext);
  def("SerializeToBytes_EvalAutomorphismKey_CryptoContext",
      &SerializeToBytes_EvalAutomorphismKey_CryptoContext);

  def("DeserializeFromBytes_EvalMultKey_CryptoContext",
      &DeserializeFromBytes_EvalMultKey_CryptoContext);
  def("DeserializeFromBytes_EvalAutomorphismKey_CryptoContext",
      &DeserializeFromBytes_EvalAutomorphismKey_CryptoContext);
}

} // namespace pyOpenFHE_CKKS