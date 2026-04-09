#include "pqc_module.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    if (pqc_set_algorithm((pqc_algorithm_t)999) != PQC_ERR_INVALID_ARG) {
        fprintf(stderr, "invalid algorithm should fail\n");
        return 1;
    }

    if (pqc_set_algorithm(PQC_ALG_ML_KEM_768_REF) != PQC_OK) {
        fprintf(stderr, "setting ref backend should succeed\n");
        return 1;
    }
    if (strstr(pqc_get_algorithm_name(), "NOT-CONNECTED") != NULL) {
        if (pqc_kem_keypair(NULL, 0, NULL, 0) != PQC_ERR_INTERNAL) {
            fprintf(stderr, "ref placeholder should report internal error\n");
            return 1;
        }
    } else {
        size_t pk, sk;
        uint8_t *pub;
        uint8_t *sec;
        pqc_status_t st;
        pqc_get_sizes(&pk, &sk, NULL, NULL);
        pub = (uint8_t *)malloc(pk);
        sec = (uint8_t *)malloc(sk);
        if (pub == NULL || sec == NULL) {
            fprintf(stderr, "allocation failed\n");
            return 1;
        }
        st = pqc_kem_keypair(pub, pk, sec, sk);
        if (st != PQC_OK) {
            fprintf(stderr, "connected ref backend keypair failed\n");
            return 1;
        }
        secure_memzero(sec, sk);
        free(pub);
        free(sec);
    }

    if (pqc_set_algorithm(PQC_ALG_ML_KEM_768_DUMMY) != PQC_OK) {
        fprintf(stderr, "failed to restore dummy backend\n");
        return 1;
    }

    printf("negative tests passed\n");
    return 0;
}
