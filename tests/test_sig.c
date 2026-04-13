#include "pqc_module.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int run_sign_verify_once(void) {
    size_t pk, sk, sig_len;
    uint8_t *pub = NULL;
    uint8_t *sec = NULL;
    uint8_t *sig = NULL;
    const uint8_t msg[] = "pqc signature test message";
    pqc_status_t st;

    pqc_sig_get_sizes(&pk, &sk, &sig_len);
    pub = (uint8_t *)malloc(pk);
    sec = (uint8_t *)malloc(sk);
    sig = (uint8_t *)malloc(sig_len);
    if (pub == NULL || sec == NULL || sig == NULL) {
        fprintf(stderr, "allocation failed\n");
        return 1;
    }

    st = pqc_sig_keypair(pub, pk, sec, sk);
    if (st != PQC_OK) {
        fprintf(stderr, "sig keypair failed\n");
        return 1;
    }

    st = pqc_sig_sign(sig, sig_len, msg, sizeof(msg) - 1, sec, sk);
    if (st != PQC_OK) {
        fprintf(stderr, "sig sign failed\n");
        return 1;
    }

    st = pqc_sig_verify(sig, sig_len, msg, sizeof(msg) - 1, pub, pk);
    if (st != PQC_OK) {
        fprintf(stderr, "sig verify failed\n");
        return 1;
    }

    sig[sig_len - 1] ^= 0x01;
    st = pqc_sig_verify(sig, sig_len, msg, sizeof(msg) - 1, pub, pk);
    if (st != PQC_ERR_VERIFY_FAILED) {
        fprintf(stderr, "tampered signature should fail\n");
        return 1;
    }

    secure_memzero(sec, sk);
    secure_memzero(sig, sig_len);
    free(pub);
    free(sec);
    free(sig);

    return 0;
}

int main(void) {
    if (pqc_set_signature_algorithm(PQC_SIG_ALG_ML_DSA_65_DUMMY) != PQC_OK) {
        fprintf(stderr, "failed to set dummy sig algorithm\n");
        return 1;
    }
    if (run_sign_verify_once() != 0) {
        return 1;
    }

    if (pqc_set_signature_algorithm(PQC_SIG_ALG_ML_DSA_65_REF) == PQC_OK) {
        if (run_sign_verify_once() != 0) {
            return 1;
        }
    }

    printf("sig tests passed\n");
    return 0;
}
