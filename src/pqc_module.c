#include "pqc_kem_backend.h"
#include "pqc_module.h"

pqc_status_t pqc_set_algorithm(pqc_algorithm_t algorithm) {
    return pqc_kem_set_backend(algorithm);
}

pqc_algorithm_t pqc_get_algorithm(void) {
    return pqc_kem_get_algorithm();
}

const char *pqc_get_algorithm_name(void) {
    return pqc_kem_get_backend()->name;
}

void pqc_get_sizes(size_t *public_key_size,
                   size_t *secret_key_size,
                   size_t *ciphertext_size,
                   size_t *shared_secret_size) {
    const pqc_kem_backend_t *backend = pqc_kem_get_backend();
    if (public_key_size != 0) {
        *public_key_size = backend->public_key_size;
    }
    if (secret_key_size != 0) {
        *secret_key_size = backend->secret_key_size;
    }
    if (ciphertext_size != 0) {
        *ciphertext_size = backend->ciphertext_size;
    }
    if (shared_secret_size != 0) {
        *shared_secret_size = backend->shared_secret_size;
    }
}

pqc_status_t pqc_kem_keypair(uint8_t *public_key,
                             size_t public_key_len,
                             uint8_t *secret_key,
                             size_t secret_key_len) {
    return pqc_kem_get_backend()->keypair(public_key, public_key_len, secret_key, secret_key_len);
}

pqc_status_t pqc_kem_encaps(uint8_t *ciphertext,
                            size_t ciphertext_len,
                            uint8_t *shared_secret,
                            size_t shared_secret_len,
                            const uint8_t *public_key,
                            size_t public_key_len) {
    return pqc_kem_get_backend()->encaps(ciphertext,
                                         ciphertext_len,
                                         shared_secret,
                                         shared_secret_len,
                                         public_key,
                                         public_key_len);
}

pqc_status_t pqc_kem_decaps(uint8_t *shared_secret,
                            size_t shared_secret_len,
                            const uint8_t *ciphertext,
                            size_t ciphertext_len,
                            const uint8_t *secret_key,
                            size_t secret_key_len) {
    return pqc_kem_get_backend()->decaps(shared_secret,
                                         shared_secret_len,
                                         ciphertext,
                                         ciphertext_len,
                                         secret_key,
                                         secret_key_len);
}
