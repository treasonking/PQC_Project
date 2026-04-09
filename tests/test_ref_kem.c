#include "pqc_module.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int run_ref_roundtrip(void) {
    size_t pk, sk, ct, ss;
    uint8_t *public_key = NULL;
    uint8_t *secret_key = NULL;
    uint8_t *ciphertext = NULL;
    uint8_t *ss1 = NULL;
    uint8_t *ss2 = NULL;
    pqc_status_t st;
    int iter;

    st = pqc_set_algorithm(PQC_ALG_ML_KEM_768_REF);
    if (st != PQC_OK) {
        fprintf(stderr, "failed to select ref algorithm\n");
        return 0;
    }

    if (strstr(pqc_get_algorithm_name(), "NOT-CONNECTED") != NULL) {
        printf("ref backend not connected, skip\n");
        return 1;
    }

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

    for (iter = 0; iter < 100; ++iter) {
        st = pqc_kem_keypair(public_key, pk, secret_key, sk);
        if (st != PQC_OK) {
            fprintf(stderr, "ref keypair failed\n");
            return 0;
        }
        st = pqc_kem_encaps(ciphertext, ct, ss1, ss, public_key, pk);
        if (st != PQC_OK) {
            fprintf(stderr, "ref encaps failed\n");
            return 0;
        }
        st = pqc_kem_decaps(ss2, ss, ciphertext, ct, secret_key, sk);
        if (st != PQC_OK) {
            fprintf(stderr, "ref decaps failed\n");
            return 0;
        }
        if (memcmp(ss1, ss2, ss) != 0) {
            fprintf(stderr, "ref shared secret mismatch at iter %d\n", iter);
            return 0;
        }
    }

    secure_memzero(secret_key, sk);
    secure_memzero(ss1, ss);
    secure_memzero(ss2, ss);
    free(public_key);
    free(secret_key);
    free(ciphertext);
    free(ss1);
    free(ss2);

    st = pqc_set_algorithm(PQC_ALG_ML_KEM_768_DUMMY);
    if (st != PQC_OK) {
        fprintf(stderr, "failed to restore dummy algorithm\n");
        return 0;
    }

    return 1;
}

int main(void) {
    if (!run_ref_roundtrip()) {
        return 1;
    }
    printf("ref tests passed\n");
    return 0;
}
