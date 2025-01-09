// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

/*
define all the serialization functions which operate on our wrapped ciphertext,
crypto context, and keys here
*/

#include <stdexcept>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "bgv/BGV_ciphertext_extension.hpp"
#include "bgv/BGV_key_operations.hpp"
#include "bgv/serialization.hpp"
#include "utils/enums_binding.hpp"
#include "utils/utils.hpp"

// header files needed for serialization
#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "key/key-ser.h"
#include "openfhe.h"
#include "scheme/bgvrns/bgvrns-ser.h"

using namespace lbcrypto;

namespace pyOpenFHE_BGV {

PyObject *SerializeToBytes_Ciphertext(const pyOpenFHE_BGV::BGVCiphertext &obj,
                                      const pyOpenFHE_BGV::SerType sertype) {
  std::stringstream ss;

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    Serial::Serialize(obj.cipher, ss, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    Serial::Serialize(obj.cipher, ss, lbcrypto::SerType::JSON);
  }

  std::string result = ss.str();
  PyObject *pymemview = PyMemoryView_FromMemory((char *)result.c_str(),
                                                result.length(), PyBUF_READ);
  return PyBytes_FromObject(pymemview);
}

pyOpenFHE_BGV::BGVCiphertext
DeserializeFromBytes_Ciphertext(boost::python::object py_buffer,
                                const pyOpenFHE_BGV::SerType sertype) {
  std::string object_classname = boost::python::extract<std::string>(
      py_buffer.attr("__class__").attr("__name__"));
  if (object_classname != "bytes") {
    throw std::runtime_error(
        "expected object of type bytes, instead received type: " +
        object_classname);
  }

  std::string buffer = boost::python::extract<std::string>(py_buffer);
  std::stringstream ss(buffer);

  Ciphertext<DCRTPoly> obj;

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    Serial::Deserialize(obj, ss, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    Serial::Deserialize(obj, ss, lbcrypto::SerType::JSON);
  }

  return pyOpenFHE_BGV::BGVCiphertext(obj);
}

PyObject *SerializeToBytes_PublicKey(const PublicKey<DCRTPoly> &obj,
                                     const pyOpenFHE_BGV::SerType sertype) {
  std::stringstream ss;

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    Serial::Serialize(obj, ss, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    Serial::Serialize(obj, ss, lbcrypto::SerType::JSON);
  }

  std::string result = ss.str();
  PyObject *pymemview = PyMemoryView_FromMemory((char *)result.c_str(),
                                                result.length(), PyBUF_READ);
  return PyBytes_FromObject(pymemview);
}

PublicKey<DCRTPoly>
DeserializeFromBytes_PublicKey(boost::python::object py_buffer,
                               const pyOpenFHE_BGV::SerType sertype) {
  std::string object_classname = boost::python::extract<std::string>(
      py_buffer.attr("__class__").attr("__name__"));
  if (object_classname != "bytes") {
    throw std::runtime_error(
        "expected object of type bytes, instead received type: " +
        object_classname);
  }

  std::string buffer = boost::python::extract<std::string>(py_buffer);
  std::stringstream ss(buffer);

  PublicKey<DCRTPoly> obj;

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    Serial::Deserialize(obj, ss, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    Serial::Deserialize(obj, ss, lbcrypto::SerType::JSON);
  }

  return obj;
}

PyObject *SerializeToBytes_PrivateKey(const PrivateKey<DCRTPoly> &obj,
                                      const pyOpenFHE_BGV::SerType sertype) {
  std::stringstream ss;

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    Serial::Serialize(obj, ss, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    Serial::Serialize(obj, ss, lbcrypto::SerType::JSON);
  }

  std::string result = ss.str();
  PyObject *pymemview = PyMemoryView_FromMemory((char *)result.c_str(),
                                                result.length(), PyBUF_READ);
  return PyBytes_FromObject(pymemview);
}

PrivateKey<DCRTPoly>
DeserializeFromBytes_PrivateKey(boost::python::object py_buffer,
                                const pyOpenFHE_BGV::SerType sertype) {
  std::string object_classname = boost::python::extract<std::string>(
      py_buffer.attr("__class__").attr("__name__"));
  if (object_classname != "bytes") {
    throw std::runtime_error(
        "expected object of type bytes, instead received type: " +
        object_classname);
  }

  std::string buffer = boost::python::extract<std::string>(py_buffer);
  std::stringstream ss(buffer);

  PrivateKey<DCRTPoly> obj;

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    Serial::Deserialize(obj, ss, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    Serial::Deserialize(obj, ss, lbcrypto::SerType::JSON);
  }

  return obj;
}

PyObject *SerializeToBytes_EvalMultKey_CryptoContext(
    BGVCryptoContext &self, const pyOpenFHE_BGV::SerType sertype) {
  std::stringstream ss;

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    self.context->SerializeEvalMultKey(ss, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    self.context->SerializeEvalMultKey(ss, lbcrypto::SerType::JSON);
  }

  std::string result = ss.str();
  PyObject *pymemview = PyMemoryView_FromMemory((char *)result.c_str(),
                                                result.length(), PyBUF_READ);
  return PyBytes_FromObject(pymemview);
}

bool DeserializeFromBytes_EvalMultKey_CryptoContext(
    BGVCryptoContext &self, boost::python::object py_buffer,
    const pyOpenFHE_BGV::SerType sertype) {
  std::string object_classname = boost::python::extract<std::string>(
      py_buffer.attr("__class__").attr("__name__"));
  if (object_classname != "bytes") {
    throw std::runtime_error(
        "expected object of type bytes, instead received type: " +
        object_classname);
  }

  std::string buffer = boost::python::extract<std::string>(py_buffer);
  std::stringstream ss(buffer);

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    self.context->DeserializeEvalMultKey(ss, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    self.context->DeserializeEvalMultKey(ss, lbcrypto::SerType::JSON);
  }

  return true;
}

PyObject *SerializeToBytes_EvalAutomorphismKey_CryptoContext(
    BGVCryptoContext &self, const pyOpenFHE_BGV::SerType sertype) {
  std::stringstream ss;

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    self.context->SerializeEvalAutomorphismKey(ss, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    self.context->SerializeEvalAutomorphismKey(ss, lbcrypto::SerType::JSON);
  }

  std::string result = ss.str();
  PyObject *pymemview = PyMemoryView_FromMemory((char *)result.c_str(),
                                                result.length(), PyBUF_READ);
  return PyBytes_FromObject(pymemview);
}

bool DeserializeFromBytes_EvalAutomorphismKey_CryptoContext(
    BGVCryptoContext &self, boost::python::object py_buffer,
    const pyOpenFHE_BGV::SerType sertype) {
  std::string object_classname = boost::python::extract<std::string>(
      py_buffer.attr("__class__").attr("__name__"));
  if (object_classname != "bytes") {
    throw std::runtime_error(
        "expected object of type bytes, instead received type: " +
        object_classname);
  }

  std::string buffer = boost::python::extract<std::string>(py_buffer);
  std::stringstream ss(buffer);

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    self.context->DeserializeEvalAutomorphismKey(ss, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    self.context->DeserializeEvalAutomorphismKey(ss, lbcrypto::SerType::JSON);
  }

  return true;
}

bool SerializeToFile_Ciphertext(const std::string &filename,
                                const pyOpenFHE_BGV::BGVCiphertext &obj,
                                const pyOpenFHE_BGV::SerType sertype) {
  bool success = false;
  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    success = Serial::SerializeToFile(filename, obj.cipher,
                                      lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    success =
        Serial::SerializeToFile(filename, obj.cipher, lbcrypto::SerType::JSON);
  }

  if (!success) {
    throw std::runtime_error(
        "Could not write serialized BGVCiphertext to file: ");
  }
  return success;
}

bool SerializeToFile_CryptoContext(const std::string &filename,
                                   const BGVCryptoContext &obj,
                                   const pyOpenFHE_BGV::SerType sertype) {
  // throw std::runtime_error("This function is disabled as CryptoContext
  // Deserialization is broken.");

  bool success = false;
  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    success = Serial::SerializeToFile(filename, obj, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    success = Serial::SerializeToFile(filename, obj, lbcrypto::SerType::JSON);
  }

  if (!success) {
    throw std::runtime_error(
        "Could not write serialized CryptoContext to file: " + filename);
  }
  return success;
}

bool SerializeToFile_EvalMultKey_CryptoContext(
    BGVCryptoContext &self, const std::string &filename,
    const pyOpenFHE_BGV::SerType sertype) {
  std::ofstream multKeyFile(filename, std::ios::out | std::ios::binary);
  bool success = false;

  if (!multKeyFile.is_open()) {
    throw std::runtime_error(
        "Could not write serialized EvalMult / relinearization keys to file: " +
        filename);
  }

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    success = self.context->SerializeEvalMultKey(multKeyFile,
                                                 lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    success = self.context->SerializeEvalMultKey(multKeyFile,
                                                 lbcrypto::SerType::JSON);
  }

  multKeyFile.close();

  if (!success) {
    throw std::runtime_error(
        "Could not write serialized EvalMult / relinearization keys to file: " +
        filename);
  }

  return success;
}

bool SerializeToFile_EvalAutomorphismKey_CryptoContext(
    BGVCryptoContext &self, const std::string &filename,
    const pyOpenFHE_BGV::SerType sertype) {
  std::ofstream multKeyFile(filename, std::ios::out | std::ios::binary);
  bool success = false;

  if (!multKeyFile.is_open()) {
    throw std::runtime_error("Could not write serialized EvalAutomorphism / "
                             "rotation keys to file: " +
                             filename);
  }

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    success = self.context->SerializeEvalAutomorphismKey(
        multKeyFile, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    success = self.context->SerializeEvalAutomorphismKey(
        multKeyFile, lbcrypto::SerType::JSON);
  }

  multKeyFile.close();

  if (!success) {
    throw std::runtime_error("Could not write serialized EvalAutomorphism / "
                             "rotation keys to file: " +
                             filename);
  }

  return success;
}

bool SerializeToFile_PublicKey(const std::string &filename,
                               const PublicKey<DCRTPoly> &obj,
                               const pyOpenFHE_BGV::SerType sertype) {
  bool success = false;
  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    success = Serial::SerializeToFile(filename, obj, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    success = Serial::SerializeToFile(filename, obj, lbcrypto::SerType::JSON);
  }

  if (!success) {
    throw std::runtime_error("Could not write serialized PublicKey to file: " +
                             filename);
  }
  return success;
}

bool SerializeToFile_PrivateKey(const std::string &filename,
                                const PrivateKey<DCRTPoly> &obj,
                                const pyOpenFHE_BGV::SerType sertype) {
  bool success = false;
  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    success = Serial::SerializeToFile(filename, obj, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    success = Serial::SerializeToFile(filename, obj, lbcrypto::SerType::JSON);
  }

  if (!success) {
    throw std::runtime_error("Could not write serialized PrivateKey to file: " +
                             filename);
  }
  return success;
}

pyOpenFHE_BGV::BGVCiphertext
DeserializeFromFile_Ciphertext(const std::string &filename,
                               const pyOpenFHE_BGV::SerType sertype) {
  bool success = false;
  Ciphertext<DCRTPoly> obj;

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    success =
        Serial::DeserializeFromFile(filename, obj, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    success =
        Serial::DeserializeFromFile(filename, obj, lbcrypto::SerType::JSON);
  }

  if (!success) {
    throw std::runtime_error("Could not read serialized data from file: " +
                             filename);
  }

  return pyOpenFHE_BGV::BGVCiphertext(obj);
}

CryptoContext<DCRTPoly>
DeserializeFromFile_CryptoContext(const std::string &filename,
                                  const pyOpenFHE_BGV::SerType sertype) {
  throw std::runtime_error(
      "This function is disabled as CryptoContext Deserialization is broken.");

  std::cout << "hello we started the deserialization function" << std::endl;

  lbcrypto::CryptoContextFactory<lbcrypto::DCRTPoly>::ReleaseAllContexts();

  std::cout << "contexts are released" << std::endl;

  CryptoContext<DCRTPoly> obj;
  bool success = false;

  std::cout << "new contexts is created" << std::endl;

  // obj->ClearEvalMultKeys();
  // obj->ClearEvalAutomorphismKeys();

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    success =
        Serial::DeserializeFromFile(filename, obj, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    success =
        Serial::DeserializeFromFile(filename, obj, lbcrypto::SerType::JSON);
  }

  if (!success) {
    throw std::runtime_error("Could not read serialized data from file: " +
                             filename);
  }

  std::cout << "deserialization maybe happened" << std::endl;

  auto cc = obj;
  std::cout << "BGV scheme is using ring dimension = " << cc->GetRingDimension()
            << std::endl;
  std::cout << "batch size = " << cc->GetEncodingParams()->GetBatchSize()
            << std::endl;

  return obj;
}

PublicKey<DCRTPoly>
DeserializeFromFile_PublicKey(const std::string &filename,
                              const pyOpenFHE_BGV::SerType sertype) {
  PublicKey<DCRTPoly> obj;
  bool success = false;

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    success =
        Serial::DeserializeFromFile(filename, obj, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    success =
        Serial::DeserializeFromFile(filename, obj, lbcrypto::SerType::JSON);
  }

  if (!success) {
    throw std::runtime_error("Could not read serialized data from file: " +
                             filename);
  }

  return obj;
}

PrivateKey<DCRTPoly>
DeserializeFromFile_PrivateKey(const std::string &filename,
                               const pyOpenFHE_BGV::SerType sertype) {
  PrivateKey<DCRTPoly> obj;
  bool success = false;

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    success =
        Serial::DeserializeFromFile(filename, obj, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    success =
        Serial::DeserializeFromFile(filename, obj, lbcrypto::SerType::JSON);
  }

  if (!success) {
    throw std::runtime_error("Could not read serialized data from file: " +
                             filename);
  }

  return obj;
}

bool DeserializeFromFile_EvalMultKey_CryptoContext(
    BGVCryptoContext &self, const std::string &filename,
    const pyOpenFHE_BGV::SerType sertype) {
  std::ifstream multKeyFile(filename, std::ios::in | std::ios::binary);
  bool success = false;

  if (!multKeyFile.is_open()) {
    throw std::runtime_error(
        "Error reading EvalMult / relinearization keys from file: " + filename);
  }

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    success = self.context->DeserializeEvalMultKey(multKeyFile,
                                                   lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    success = self.context->DeserializeEvalMultKey(multKeyFile,
                                                   lbcrypto::SerType::JSON);
  }

  multKeyFile.close();

  return success;
}

bool DeserializeFromFile_EvalAutomorphismKey_CryptoContext(
    BGVCryptoContext &self, const std::string &filename,
    const pyOpenFHE_BGV::SerType sertype) {
  std::ifstream multKeyFile(filename, std::ios::in | std::ios::binary);
  bool success = false;

  if (!multKeyFile.is_open()) {
    throw std::runtime_error(
        "Error reading EvalAutomorphism / rotation keys from file: " +
        filename);
  }

  if (sertype == pyOpenFHE_BGV::SerType::BINARY) {
    success = self.context->DeserializeEvalAutomorphismKey(
        multKeyFile, lbcrypto::SerType::BINARY);
  } else if (sertype == pyOpenFHE_BGV::SerType::JSON) {
    success = self.context->DeserializeEvalAutomorphismKey(
        multKeyFile, lbcrypto::SerType::JSON);
  }

  multKeyFile.close();

  return success;
}

} // namespace pyOpenFHE_BGV
