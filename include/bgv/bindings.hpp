// (c) 2021-2024 The Johns Hopkins University Applied Physics Laboratory LLC (JHU/APL).

#ifndef BGV_OPENFHE_PYTHON_BINDINGS_H
#define BGV_OPENFHE_PYTHON_BINDINGS_H

namespace pyOpenFHE_BGV {

void export_BGV_Ciphertext_boost();
void export_BGV_CryptoContext_boost();
void export_BGV_serialization_boost();

} // namespace pyOpenFHE_BGV

#endif