#include "pqc_sig_backend.h"
#include "pqc_module.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#if PQC_ENABLE_MLDSA_REF
#include "mldsa_pqclean/clean/api.h"
#endif

enum {
    DUMMY_SIG_PUBLIC_KEY_SIZE = 1312,
    DUMMY_SIG_SECRET_KEY_SIZE = 2528,
    DUMMY_SIG_SIGNATURE_SIZE = 2420
};

static int g_sig_rng_seeded = 0;
#if PQC_ENABLE_MLDSA_REF
static pqc_sig_algorithm_t g_sig_algorithm = PQC_SIG_ALG_ML_DSA_65_REF;
#else
static pqc_sig_algorithm_t g_sig_algorithm = PQC_SIG_ALG_ML_DSA_65_DUMMY;
#endif

static void ensure_rng_seeded(void) {
    if (!g_sig_rng_seeded) {
        srand((unsigned int)time(NULL));
        g_sig_rng_seeded = 1;
    }
}

static void random_bytes(uint8_t *out, size_t len) {
    size_t i;
    ensure_rng_seeded();
    for (i = 0; i < len; ++i) {
        out[i] = (uint8_t)(rand() & 0xFF);
    }
}

static uint32_t fnv1a32(const uint8_t *data, size_t len, uint32_t seed) {
    size_t i;
    uint32_t h = seed;
    for (i = 0; i < len; ++i) {
        h ^= data[i];
        h *= 16777619u;
    }
    return h;
}

static void derive_signature_bytes(uint8_t *out_sig,
                                   size_t sig_len,
                                   const uint8_t *msg,
                                   size_t msg_len,
                                   const uint8_t *pk,
                                   size_t pk_len) {
    uint32_t h;
    size_t i;
    if (sig_len < DUMMY_SIG_SIGNATURE_SIZE) {
        return;
    }

    out_sig[0] = 'D';
    out_sig[1] = 'S';
    out_sig[2] = 'I';
    out_sig[3] = 'G';

    h = fnv1a32(msg, msg_len, 2166136261u);
    h = fnv1a32(pk, pk_len, h);
    for (i = 4; i < DUMMY_SIG_SIGNATURE_SIZE; ++i) {
        h = h * 1664525u + 1013904223u + (uint32_t)i;
        out_sig[i] = (uint8_t)((h >> ((i % 4) * 8)) & 0xFF);
    }
}

static pqc_status_t dummy_sig_keypair(uint8_t *public_key,
                                      size_t public_key_len,
                                      uint8_t *secret_key,
                                      size_t secret_key_len) {
    if (public_key == NULL || secret_key == NULL) {
        return PQC_ERR_INVALID_ARG;
    }
    if (public_key_len < DUMMY_SIG_PUBLIC_KEY_SIZE || secret_key_len < DUMMY_SIG_SECRET_KEY_SIZE) {
        return PQC_ERR_BUFFER_TOO_SMALL;
    }

    random_bytes(public_key, DUMMY_SIG_PUBLIC_KEY_SIZE);
    memcpy(secret_key, public_key, DUMMY_SIG_PUBLIC_KEY_SIZE);
    random_bytes(secret_key + DUMMY_SIG_PUBLIC_KEY_SIZE, DUMMY_SIG_SECRET_KEY_SIZE - DUMMY_SIG_PUBLIC_KEY_SIZE);
    return PQC_OK;
}

static pqc_status_t dummy_sig_sign(uint8_t *signature,
                                   size_t signature_len,
                                   const uint8_t *message,
                                   size_t message_len,
                                   const uint8_t *secret_key,
                                   size_t secret_key_len) {
    const uint8_t *public_key = secret_key;
    if (signature == NULL || message == NULL || secret_key == NULL) {
        return PQC_ERR_INVALID_ARG;
    }
    if (signature_len < DUMMY_SIG_SIGNATURE_SIZE || secret_key_len < DUMMY_SIG_SECRET_KEY_SIZE) {
        return PQC_ERR_BUFFER_TOO_SMALL;
    }
    derive_signature_bytes(signature,
                           DUMMY_SIG_SIGNATURE_SIZE,
                           message,
                           message_len,
                           public_key,
                           DUMMY_SIG_PUBLIC_KEY_SIZE);
    return PQC_OK;
}

static pqc_status_t dummy_sig_verify(const uint8_t *signature,
                                     size_t signature_len,
                                     const uint8_t *message,
                                     size_t message_len,
                                     const uint8_t *public_key,
                                     size_t public_key_len) {
    uint8_t expected[DUMMY_SIG_SIGNATURE_SIZE];
    if (signature == NULL || message == NULL || public_key == NULL) {
        return PQC_ERR_INVALID_ARG;
    }
    if (signature_len < DUMMY_SIG_SIGNATURE_SIZE || public_key_len < DUMMY_SIG_PUBLIC_KEY_SIZE) {
        return PQC_ERR_BUFFER_TOO_SMALL;
    }
    derive_signature_bytes(expected,
                           DUMMY_SIG_SIGNATURE_SIZE,
                           message,
                           message_len,
                           public_key,
                           DUMMY_SIG_PUBLIC_KEY_SIZE);
    if (memcmp(signature, expected, DUMMY_SIG_SIGNATURE_SIZE) != 0) {
        return PQC_ERR_VERIFY_FAILED;
    }
    return PQC_OK;
}

#if PQC_ENABLE_MLDSA_REF
static pqc_status_t ref_mldsa_keypair(uint8_t *public_key,
                                      size_t public_key_len,
                                      uint8_t *secret_key,
                                      size_t secret_key_len) {
    if (public_key == NULL || secret_key == NULL) {
        return PQC_ERR_INVALID_ARG;
    }
    if (public_key_len < PQCLEAN_MLDSA65_CLEAN_CRYPTO_PUBLICKEYBYTES ||
        secret_key_len < PQCLEAN_MLDSA65_CLEAN_CRYPTO_SECRETKEYBYTES) {
        return PQC_ERR_BUFFER_TOO_SMALL;
    }
    if (PQCLEAN_MLDSA65_CLEAN_crypto_sign_keypair(public_key, secret_key) != 0) {
        return PQC_ERR_KEYGEN_FAILED;
    }
    return PQC_OK;
}

static pqc_status_t ref_mldsa_sign(uint8_t *signature,
                                   size_t signature_len,
                                   const uint8_t *message,
                                   size_t message_len,
                                   const uint8_t *secret_key,
                                   size_t secret_key_len) {
    size_t out_len = 0;
    if (signature == NULL || message == NULL || secret_key == NULL) {
        return PQC_ERR_INVALID_ARG;
    }
    if (signature_len < PQCLEAN_MLDSA65_CLEAN_CRYPTO_BYTES ||
        secret_key_len < PQCLEAN_MLDSA65_CLEAN_CRYPTO_SECRETKEYBYTES) {
        return PQC_ERR_BUFFER_TOO_SMALL;
    }
    if (PQCLEAN_MLDSA65_CLEAN_crypto_sign_signature(signature,
                                                    &out_len,
                                                    message,
                                                    message_len,
                                                    secret_key) != 0) {
        return PQC_ERR_SIGN_FAILED;
    }
    if (out_len != PQCLEAN_MLDSA65_CLEAN_CRYPTO_BYTES) {
        return PQC_ERR_SIGN_FAILED;
    }
    return PQC_OK;
}

static pqc_status_t ref_mldsa_verify(const uint8_t *signature,
                                     size_t signature_len,
                                     const uint8_t *message,
                                     size_t message_len,
                                     const uint8_t *public_key,
                                     size_t public_key_len) {
    if (signature == NULL || message == NULL || public_key == NULL) {
        return PQC_ERR_INVALID_ARG;
    }
    if (signature_len < PQCLEAN_MLDSA65_CLEAN_CRYPTO_BYTES ||
        public_key_len < PQCLEAN_MLDSA65_CLEAN_CRYPTO_PUBLICKEYBYTES) {
        return PQC_ERR_BUFFER_TOO_SMALL;
    }
    if (PQCLEAN_MLDSA65_CLEAN_crypto_sign_verify(signature,
                                                 signature_len,
                                                 message,
                                                 message_len,
                                                 public_key) != 0) {
        return PQC_ERR_VERIFY_FAILED;
    }
    return PQC_OK;
}
#endif

static const pqc_sig_backend_t DUMMY_SIG_BACKEND = {
    "DUMMY-ML-DSA-65-API",
    DUMMY_SIG_PUBLIC_KEY_SIZE,
    DUMMY_SIG_SECRET_KEY_SIZE,
    DUMMY_SIG_SIGNATURE_SIZE,
    dummy_sig_keypair,
    dummy_sig_sign,
    dummy_sig_verify};

#if PQC_ENABLE_MLDSA_REF
static const pqc_sig_backend_t REF_SIG_BACKEND = {
    "ML-DSA-65-PQCLEAN",
    PQCLEAN_MLDSA65_CLEAN_CRYPTO_PUBLICKEYBYTES,
    PQCLEAN_MLDSA65_CLEAN_CRYPTO_SECRETKEYBYTES,
    PQCLEAN_MLDSA65_CLEAN_CRYPTO_BYTES,
    ref_mldsa_keypair,
    ref_mldsa_sign,
    ref_mldsa_verify};
#endif

const pqc_sig_backend_t *pqc_sig_get_backend(void) {
#if PQC_ENABLE_MLDSA_REF
    if (g_sig_algorithm == PQC_SIG_ALG_ML_DSA_65_REF) {
        return &REF_SIG_BACKEND;
    }
#endif
    return &DUMMY_SIG_BACKEND;
}

pqc_status_t pqc_sig_set_backend(pqc_sig_algorithm_t algorithm) {
    if (algorithm == PQC_SIG_ALG_ML_DSA_65_DUMMY) {
        g_sig_algorithm = algorithm;
        return PQC_OK;
    }
#if PQC_ENABLE_MLDSA_REF
    if (algorithm == PQC_SIG_ALG_ML_DSA_65_REF) {
        g_sig_algorithm = algorithm;
        return PQC_OK;
    }
#endif
    return PQC_ERR_INVALID_ARG;
}

pqc_sig_algorithm_t pqc_sig_get_algorithm(void) {
    return g_sig_algorithm;
}
