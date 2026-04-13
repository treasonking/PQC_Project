#include "pqc_kem_backend.h"
#include "pqc_module.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#if PQC_ENABLE_MLKEM_REF
#include "mlkem_pqclean/clean/api.h"
#endif

enum {
    DUMMY_PUBLIC_KEY_SIZE = 800,
    DUMMY_SECRET_KEY_SIZE = 1632,
    DUMMY_CIPHERTEXT_SIZE = 768,
    DUMMY_SHARED_SECRET_SIZE = 32
};

static int g_rng_seeded = 0;
static pqc_algorithm_t g_active_algorithm = PQC_ALG_ML_KEM_768_DUMMY;

static void ensure_rng_seeded(void) {
    if (!g_rng_seeded) {
        srand((unsigned int)time(NULL));
        g_rng_seeded = 1;
    }
}

static void random_bytes(uint8_t *out, size_t len) {
    size_t i;
    ensure_rng_seeded();
    for (i = 0; i < len; ++i) {
        out[i] = (uint8_t)(rand() & 0xFF);
    }
}

static pqc_status_t dummy_keypair(uint8_t *public_key,
                                  size_t public_key_len,
                                  uint8_t *secret_key,
                                  size_t secret_key_len) {
    if (public_key == NULL || secret_key == NULL) {
        return PQC_ERR_INVALID_ARG;
    }
    if (public_key_len < DUMMY_PUBLIC_KEY_SIZE || secret_key_len < DUMMY_SECRET_KEY_SIZE) {
        return PQC_ERR_BUFFER_TOO_SMALL;
    }

    random_bytes(public_key, DUMMY_PUBLIC_KEY_SIZE);
    memcpy(secret_key, public_key, DUMMY_PUBLIC_KEY_SIZE);
    random_bytes(secret_key + DUMMY_PUBLIC_KEY_SIZE, DUMMY_SECRET_KEY_SIZE - DUMMY_PUBLIC_KEY_SIZE);
    return PQC_OK;
}

static pqc_status_t dummy_encaps(uint8_t *ciphertext,
                                 size_t ciphertext_len,
                                 uint8_t *shared_secret,
                                 size_t shared_secret_len,
                                 const uint8_t *public_key,
                                 size_t public_key_len) {
    size_t i;
    uint8_t nonce[DUMMY_SHARED_SECRET_SIZE];

    if (ciphertext == NULL || shared_secret == NULL || public_key == NULL) {
        return PQC_ERR_INVALID_ARG;
    }
    if (ciphertext_len < DUMMY_CIPHERTEXT_SIZE ||
        shared_secret_len < DUMMY_SHARED_SECRET_SIZE ||
        public_key_len < DUMMY_PUBLIC_KEY_SIZE) {
        return PQC_ERR_BUFFER_TOO_SMALL;
    }

    random_bytes(nonce, sizeof(nonce));
    memcpy(ciphertext, nonce, sizeof(nonce));

    for (i = sizeof(nonce); i < DUMMY_CIPHERTEXT_SIZE; ++i) {
        ciphertext[i] = (uint8_t)(public_key[i % DUMMY_PUBLIC_KEY_SIZE] ^ nonce[i % sizeof(nonce)] ^ 0x5A);
    }
    for (i = 0; i < DUMMY_SHARED_SECRET_SIZE; ++i) {
        shared_secret[i] = (uint8_t)(nonce[i] ^ public_key[(i * 7U) % DUMMY_PUBLIC_KEY_SIZE]);
    }

    secure_memzero(nonce, sizeof(nonce));
    return PQC_OK;
}

static pqc_status_t dummy_decaps(uint8_t *shared_secret,
                                 size_t shared_secret_len,
                                 const uint8_t *ciphertext,
                                 size_t ciphertext_len,
                                 const uint8_t *secret_key,
                                 size_t secret_key_len) {
    size_t i;
    const uint8_t *public_key;
    uint8_t expected;

    if (shared_secret == NULL || ciphertext == NULL || secret_key == NULL) {
        return PQC_ERR_INVALID_ARG;
    }
    if (shared_secret_len < DUMMY_SHARED_SECRET_SIZE ||
        ciphertext_len < DUMMY_CIPHERTEXT_SIZE ||
        secret_key_len < DUMMY_SECRET_KEY_SIZE) {
        return PQC_ERR_BUFFER_TOO_SMALL;
    }

    public_key = secret_key;
    for (i = DUMMY_SHARED_SECRET_SIZE; i < DUMMY_CIPHERTEXT_SIZE; ++i) {
        expected = (uint8_t)(public_key[i % DUMMY_PUBLIC_KEY_SIZE] ^
                             ciphertext[i % DUMMY_SHARED_SECRET_SIZE] ^
                             0x5A);
        if (ciphertext[i] != expected) {
            secure_memzero(shared_secret, shared_secret_len);
            return PQC_ERR_DECAP_FAILED;
        }
    }

    for (i = 0; i < DUMMY_SHARED_SECRET_SIZE; ++i) {
        shared_secret[i] = (uint8_t)(ciphertext[i] ^ public_key[(i * 7U) % DUMMY_PUBLIC_KEY_SIZE]);
    }
    return PQC_OK;
}

/*
 * Placeholder backend for third_party ML-KEM reference implementation wiring.
 * Real implementation should live under third_party/ and populate function pointers.
 */
static pqc_status_t ref_not_connected_keypair(uint8_t *public_key,
                                              size_t public_key_len,
                                              uint8_t *secret_key,
                                              size_t secret_key_len) {
    (void)public_key;
    (void)public_key_len;
    (void)secret_key;
    (void)secret_key_len;
    return PQC_ERR_INTERNAL;
}

static pqc_status_t ref_not_connected_encaps(uint8_t *ciphertext,
                                             size_t ciphertext_len,
                                             uint8_t *shared_secret,
                                             size_t shared_secret_len,
                                             const uint8_t *public_key,
                                             size_t public_key_len) {
    (void)ciphertext;
    (void)ciphertext_len;
    (void)shared_secret;
    (void)shared_secret_len;
    (void)public_key;
    (void)public_key_len;
    return PQC_ERR_INTERNAL;
}

static pqc_status_t ref_not_connected_decaps(uint8_t *shared_secret,
                                             size_t shared_secret_len,
                                             const uint8_t *ciphertext,
                                             size_t ciphertext_len,
                                             const uint8_t *secret_key,
                                             size_t secret_key_len) {
    (void)shared_secret;
    (void)shared_secret_len;
    (void)ciphertext;
    (void)ciphertext_len;
    (void)secret_key;
    (void)secret_key_len;
    return PQC_ERR_INTERNAL;
}

#if PQC_ENABLE_MLKEM_REF
static pqc_status_t ref_mlkem_keypair(uint8_t *public_key,
                                      size_t public_key_len,
                                      uint8_t *secret_key,
                                      size_t secret_key_len) {
    if (public_key == NULL || secret_key == NULL) {
        return PQC_ERR_INVALID_ARG;
    }
    if (public_key_len < PQCLEAN_MLKEM768_CLEAN_CRYPTO_PUBLICKEYBYTES ||
        secret_key_len < PQCLEAN_MLKEM768_CLEAN_CRYPTO_SECRETKEYBYTES) {
        return PQC_ERR_BUFFER_TOO_SMALL;
    }
    if (PQCLEAN_MLKEM768_CLEAN_crypto_kem_keypair(public_key, secret_key) != 0) {
        return PQC_ERR_KEYGEN_FAILED;
    }
    return PQC_OK;
}

static pqc_status_t ref_mlkem_encaps(uint8_t *ciphertext,
                                     size_t ciphertext_len,
                                     uint8_t *shared_secret,
                                     size_t shared_secret_len,
                                     const uint8_t *public_key,
                                     size_t public_key_len) {
    if (ciphertext == NULL || shared_secret == NULL || public_key == NULL) {
        return PQC_ERR_INVALID_ARG;
    }
    if (ciphertext_len < PQCLEAN_MLKEM768_CLEAN_CRYPTO_CIPHERTEXTBYTES ||
        shared_secret_len < PQCLEAN_MLKEM768_CLEAN_CRYPTO_BYTES ||
        public_key_len < PQCLEAN_MLKEM768_CLEAN_CRYPTO_PUBLICKEYBYTES) {
        return PQC_ERR_BUFFER_TOO_SMALL;
    }
    if (PQCLEAN_MLKEM768_CLEAN_crypto_kem_enc(ciphertext, shared_secret, public_key) != 0) {
        secure_memzero(shared_secret, shared_secret_len);
        return PQC_ERR_ENCAP_FAILED;
    }
    return PQC_OK;
}

static pqc_status_t ref_mlkem_decaps(uint8_t *shared_secret,
                                     size_t shared_secret_len,
                                     const uint8_t *ciphertext,
                                     size_t ciphertext_len,
                                     const uint8_t *secret_key,
                                     size_t secret_key_len) {
    if (shared_secret == NULL || ciphertext == NULL || secret_key == NULL) {
        return PQC_ERR_INVALID_ARG;
    }
    if (shared_secret_len < PQCLEAN_MLKEM768_CLEAN_CRYPTO_BYTES ||
        ciphertext_len < PQCLEAN_MLKEM768_CLEAN_CRYPTO_CIPHERTEXTBYTES ||
        secret_key_len < PQCLEAN_MLKEM768_CLEAN_CRYPTO_SECRETKEYBYTES) {
        return PQC_ERR_BUFFER_TOO_SMALL;
    }
    if (PQCLEAN_MLKEM768_CLEAN_crypto_kem_dec(shared_secret, ciphertext, secret_key) != 0) {
        secure_memzero(shared_secret, shared_secret_len);
        return PQC_ERR_DECAP_FAILED;
    }
    return PQC_OK;
}
#endif

static const pqc_kem_backend_t DUMMY_BACKEND = {
    "DUMMY-ML-KEM-API",
    DUMMY_PUBLIC_KEY_SIZE,
    DUMMY_SECRET_KEY_SIZE,
    DUMMY_CIPHERTEXT_SIZE,
    DUMMY_SHARED_SECRET_SIZE,
    dummy_keypair,
    dummy_encaps,
    dummy_decaps};

#if PQC_ENABLE_MLKEM_REF
static const pqc_kem_backend_t REF_BACKEND = {
    "ML-KEM-768-PQCLEAN",
    PQCLEAN_MLKEM768_CLEAN_CRYPTO_PUBLICKEYBYTES,
    PQCLEAN_MLKEM768_CLEAN_CRYPTO_SECRETKEYBYTES,
    PQCLEAN_MLKEM768_CLEAN_CRYPTO_CIPHERTEXTBYTES,
    PQCLEAN_MLKEM768_CLEAN_CRYPTO_BYTES,
    ref_mlkem_keypair,
    ref_mlkem_encaps,
    ref_mlkem_decaps};
#else
static const pqc_kem_backend_t REF_BACKEND_PLACEHOLDER = {
    "ML-KEM-REF-NOT-CONNECTED",
    DUMMY_PUBLIC_KEY_SIZE,
    DUMMY_SECRET_KEY_SIZE,
    DUMMY_CIPHERTEXT_SIZE,
    DUMMY_SHARED_SECRET_SIZE,
    ref_not_connected_keypair,
    ref_not_connected_encaps,
    ref_not_connected_decaps};
#endif

const pqc_kem_backend_t *pqc_kem_get_backend(void) {
    if (g_active_algorithm == PQC_ALG_ML_KEM_768_REF) {
#if PQC_ENABLE_MLKEM_REF
        return &REF_BACKEND;
#else
        return &REF_BACKEND_PLACEHOLDER;
#endif
    }
    return &DUMMY_BACKEND;
}

pqc_status_t pqc_kem_set_backend(pqc_algorithm_t algorithm) {
    if (algorithm != PQC_ALG_ML_KEM_768_DUMMY && algorithm != PQC_ALG_ML_KEM_768_REF) {
        return PQC_ERR_INVALID_ARG;
    }
    g_active_algorithm = algorithm;
    return PQC_OK;
}

pqc_algorithm_t pqc_kem_get_algorithm(void) {
    return g_active_algorithm;
}
