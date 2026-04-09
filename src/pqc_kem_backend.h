#ifndef PQC_KEM_BACKEND_H
#define PQC_KEM_BACKEND_H

#include "pqc_module.h"

typedef struct {
    const char *name;
    size_t public_key_size;
    size_t secret_key_size;
    size_t ciphertext_size;
    size_t shared_secret_size;
    pqc_status_t (*keypair)(uint8_t *public_key,
                            size_t public_key_len,
                            uint8_t *secret_key,
                            size_t secret_key_len);
    pqc_status_t (*encaps)(uint8_t *ciphertext,
                           size_t ciphertext_len,
                           uint8_t *shared_secret,
                           size_t shared_secret_len,
                           const uint8_t *public_key,
                           size_t public_key_len);
    pqc_status_t (*decaps)(uint8_t *shared_secret,
                           size_t shared_secret_len,
                           const uint8_t *ciphertext,
                           size_t ciphertext_len,
                           const uint8_t *secret_key,
                           size_t secret_key_len);
} pqc_kem_backend_t;

const pqc_kem_backend_t *pqc_kem_get_backend(void);
pqc_status_t pqc_kem_set_backend(pqc_algorithm_t algorithm);
pqc_algorithm_t pqc_kem_get_algorithm(void);

#endif
