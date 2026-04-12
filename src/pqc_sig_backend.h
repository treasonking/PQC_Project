#ifndef PQC_SIG_BACKEND_H
#define PQC_SIG_BACKEND_H

#include "pqc_module.h"

typedef struct {
    const char *name;
    size_t public_key_size;
    size_t secret_key_size;
    size_t signature_size;
    pqc_status_t (*keypair)(uint8_t *public_key,
                            size_t public_key_len,
                            uint8_t *secret_key,
                            size_t secret_key_len);
    pqc_status_t (*sign)(uint8_t *signature,
                         size_t signature_len,
                         const uint8_t *message,
                         size_t message_len,
                         const uint8_t *secret_key,
                         size_t secret_key_len);
    pqc_status_t (*verify)(const uint8_t *signature,
                           size_t signature_len,
                           const uint8_t *message,
                           size_t message_len,
                           const uint8_t *public_key,
                           size_t public_key_len);
} pqc_sig_backend_t;

const pqc_sig_backend_t *pqc_sig_get_backend(void);
pqc_status_t pqc_sig_set_backend(pqc_sig_algorithm_t algorithm);
pqc_sig_algorithm_t pqc_sig_get_algorithm(void);

#endif
