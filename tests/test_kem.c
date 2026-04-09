#include "pqc_module.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_dummy_kem_roundtrip(void) {
    size_t pk, sk, ct, ss;
    uint8_t *public_key = NULL;
    uint8_t *secret_key = NULL;
    uint8_t *ciphertext = NULL;
    uint8_t *ss1 = NULL;
    uint8_t *ss2 = NULL;
    pqc_status_t st;

    pqc_get_sizes(&pk, &sk, &ct, &ss);
    public_key = (uint8_t *)malloc(pk);
    secret_key = (uint8_t *)malloc(sk);
    ciphertext = (uint8_t *)malloc(ct);
    ss1 = (uint8_t *)malloc(ss);
    ss2 = (uint8_t *)malloc(ss);
    if (!public_key || !secret_key || !ciphertext || !ss1 || !ss2) {
        fprintf(stderr, "allocation failed\n");
        return 0;
    }

    for (int iter = 0; iter < 100; ++iter) {
        st = pqc_kem_keypair(public_key, pk, secret_key, sk);
        if (st != PQC_OK) {
            fprintf(stderr, "keypair failed: %s\n", pqc_status_to_string(st));
            return 0;
        }
        st = pqc_kem_encaps(ciphertext, ct, ss1, ss, public_key, pk);
        if (st != PQC_OK) {
            fprintf(stderr, "encaps failed: %s\n", pqc_status_to_string(st));
            return 0;
        }
        st = pqc_kem_decaps(ss2, ss, ciphertext, ct, secret_key, sk);
        if (st != PQC_OK) {
            fprintf(stderr, "decaps failed: %s\n", pqc_status_to_string(st));
            return 0;
        }
        if (memcmp(ss1, ss2, ss) != 0) {
            fprintf(stderr, "shared secret mismatch on iter %d\n", iter);
            return 0;
        }
    }

    ciphertext[ct - 1] ^= 0x01;
    st = pqc_kem_decaps(ss2, ss, ciphertext, ct, secret_key, sk);
    if (st != PQC_ERR_DECAP_FAILED) {
        fprintf(stderr, "tamper check should fail\n");
        return 0;
    }

    secure_memzero(secret_key, sk);
    secure_memzero(ss1, ss);
    secure_memzero(ss2, ss);
    free(public_key);
    free(secret_key);
    free(ciphertext);
    free(ss1);
    free(ss2);
    return 1;
}

static int test_invalid_args(void) {
    size_t pk, sk, ct, ss;
    uint8_t dummy[8] = {0};

    pqc_get_sizes(&pk, &sk, &ct, &ss);

    if (pqc_kem_keypair(NULL, pk, dummy, sk) != PQC_ERR_INVALID_ARG) {
        return 0;
    }
    if (pqc_kem_keypair(dummy, pk, NULL, sk) != PQC_ERR_INVALID_ARG) {
        return 0;
    }
    if (pqc_kem_keypair(dummy, pk - 1, dummy, sk) != PQC_ERR_BUFFER_TOO_SMALL) {
        return 0;
    }
    if (pqc_kem_encaps(NULL, ct, dummy, ss, dummy, pk) != PQC_ERR_INVALID_ARG) {
        return 0;
    }
    if (pqc_kem_decaps(dummy, ss, NULL, ct, dummy, sk) != PQC_ERR_INVALID_ARG) {
        return 0;
    }

    return 1;
}

int main(void) {
    if (!test_dummy_kem_roundtrip()) {
        return 1;
    }
    if (!test_invalid_args()) {
        fprintf(stderr, "invalid arg test failed\n");
        return 1;
    }

    printf("all tests passed\n");
    return 0;
}
