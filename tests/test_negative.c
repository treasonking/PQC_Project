#include "pqc_module.h"

#include <stdio.h>

int main(void) {
    if (pqc_set_algorithm((pqc_algorithm_t)999) != PQC_ERR_INVALID_ARG) {
        fprintf(stderr, "invalid algorithm should fail\n");
        return 1;
    }

    if (pqc_set_algorithm(PQC_ALG_ML_KEM_768_REF) != PQC_OK) {
        fprintf(stderr, "setting ref backend should succeed\n");
        return 1;
    }
    if (pqc_kem_keypair(NULL, 0, NULL, 0) != PQC_ERR_INTERNAL) {
        fprintf(stderr, "ref placeholder should report internal error\n");
        return 1;
    }

    if (pqc_set_algorithm(PQC_ALG_ML_KEM_768_DUMMY) != PQC_OK) {
        fprintf(stderr, "failed to restore dummy backend\n");
        return 1;
    }

    printf("negative tests passed\n");
    return 0;
}
