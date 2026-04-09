#ifndef PQC_MODULE_H
#define PQC_MODULE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PQC_OK = 0,
    PQC_ERR_INVALID_ARG,
    PQC_ERR_BUFFER_TOO_SMALL,
    PQC_ERR_KEYGEN_FAILED,
    PQC_ERR_ENCAP_FAILED,
    PQC_ERR_DECAP_FAILED,
    PQC_ERR_VERIFY_FAILED,
    PQC_ERR_INTERNAL
} pqc_status_t;

typedef enum {
    PQC_ALG_ML_KEM_768_DUMMY = 0,
    PQC_ALG_ML_KEM_768_REF = 1
} pqc_algorithm_t;

/* Select the active KEM backend (default: PQC_ALG_ML_KEM_768_DUMMY). */
pqc_status_t pqc_set_algorithm(pqc_algorithm_t algorithm);
pqc_algorithm_t pqc_get_algorithm(void);

const char *pqc_get_algorithm_name(void);
void pqc_get_sizes(size_t *public_key_size,
                   size_t *secret_key_size,
                   size_t *ciphertext_size,
                   size_t *shared_secret_size);

pqc_status_t pqc_kem_keypair(uint8_t *public_key,
                             size_t public_key_len,
                             uint8_t *secret_key,
                             size_t secret_key_len);

pqc_status_t pqc_kem_encaps(uint8_t *ciphertext,
                            size_t ciphertext_len,
                            uint8_t *shared_secret,
                            size_t shared_secret_len,
                            const uint8_t *public_key,
                            size_t public_key_len);

pqc_status_t pqc_kem_decaps(uint8_t *shared_secret,
                            size_t shared_secret_len,
                            const uint8_t *ciphertext,
                            size_t ciphertext_len,
                            const uint8_t *secret_key,
                            size_t secret_key_len);

void secure_memzero(void *ptr, size_t len);
const char *pqc_status_to_string(pqc_status_t status);

#ifdef __cplusplus
}
#endif

#endif
