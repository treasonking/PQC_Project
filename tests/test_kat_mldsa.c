#include "pqc_module.h"

#include <stdio.h>
#include <stdlib.h>

static int read_file(const char *path, uint8_t **data, size_t *len) {
    FILE *fp;
    long sz;
    uint8_t *buf;
    size_t n;
    if (path == NULL || data == NULL || len == NULL) {
        return 0;
    }
    *data = NULL;
    *len = 0;
    fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "open failed: %s\n", path);
        return 0;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    sz = ftell(fp);
    if (sz < 0) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    buf = (uint8_t *)malloc((size_t)sz);
    if (buf == NULL && sz > 0) {
        fclose(fp);
        return 0;
    }
    n = fread(buf, 1, (size_t)sz, fp);
    fclose(fp);
    if (n != (size_t)sz) {
        free(buf);
        return 0;
    }
    *data = buf;
    *len = (size_t)sz;
    return 1;
}

int main(void) {
    char msg_path[512];
    char pk_path[512];
    char sk_path[512];
    char sig_path[512];
    uint8_t *msg = NULL;
    uint8_t *pk = NULL;
    uint8_t *sk = NULL;
    uint8_t *sig = NULL;
    uint8_t *generated_sig = NULL;
    size_t msg_len = 0;
    size_t pk_len = 0;
    size_t sk_len = 0;
    size_t sig_len = 0;
    size_t expect_pk = 0;
    size_t expect_sk = 0;
    size_t expect_sig = 0;
    pqc_status_t st;
    int rc = 1;

#ifndef TESTS_DATA_DIR
#define TESTS_DATA_DIR "tests/data"
#endif

    (void)snprintf(msg_path, sizeof(msg_path), "%s/kat_mldsa_msg.txt", TESTS_DATA_DIR);
    (void)snprintf(pk_path, sizeof(pk_path), "%s/kat_mldsa_pub.key", TESTS_DATA_DIR);
    (void)snprintf(sk_path, sizeof(sk_path), "%s/kat_mldsa_sec.key", TESTS_DATA_DIR);
    (void)snprintf(sig_path, sizeof(sig_path), "%s/kat_mldsa.sig", TESTS_DATA_DIR);

    if (pqc_set_signature_algorithm(PQC_SIG_ALG_ML_DSA_65_REF) != PQC_OK) {
        printf("mldsa ref backend unavailable, skip\n");
        return 0;
    }

    pqc_sig_get_sizes(&expect_pk, &expect_sk, &expect_sig);
    if (!read_file(msg_path, &msg, &msg_len) ||
        !read_file(pk_path, &pk, &pk_len) ||
        !read_file(sk_path, &sk, &sk_len) ||
        !read_file(sig_path, &sig, &sig_len)) {
        fprintf(stderr, "failed to read KAT files\n");
        goto cleanup;
    }

    if (pk_len != expect_pk || sk_len != expect_sk || sig_len != expect_sig) {
        fprintf(stderr, "KAT size mismatch\n");
        goto cleanup;
    }

    generated_sig = (uint8_t *)malloc(expect_sig);
    if (generated_sig == NULL) {
        fprintf(stderr, "allocation failed\n");
        goto cleanup;
    }

    st = pqc_sig_verify(sig, sig_len, msg, msg_len, pk, pk_len);
    if (st != PQC_OK) {
        fprintf(stderr, "KAT verify failed\n");
        goto cleanup;
    }

    st = pqc_sig_sign(generated_sig, expect_sig, msg, msg_len, sk, sk_len);
    if (st != PQC_OK) {
        fprintf(stderr, "KAT sign with stored secret key failed\n");
        goto cleanup;
    }

    st = pqc_sig_verify(generated_sig, expect_sig, msg, msg_len, pk, pk_len);
    if (st != PQC_OK) {
        fprintf(stderr, "generated KAT signature verify failed\n");
        goto cleanup;
    }

    sig[0] ^= 0x01;
    st = pqc_sig_verify(sig, sig_len, msg, msg_len, pk, pk_len);
    if (st != PQC_ERR_VERIFY_FAILED) {
        fprintf(stderr, "tampered KAT signature should fail\n");
        goto cleanup;
    }

    pk[0] ^= 0x01;
    st = pqc_sig_verify(generated_sig, expect_sig, msg, msg_len, pk, pk_len);
    if (st != PQC_ERR_VERIFY_FAILED) {
        fprintf(stderr, "tampered KAT public key should fail\n");
        goto cleanup;
    }

    rc = 0;
cleanup:
    secure_memzero(sk, sk_len);
    secure_memzero(generated_sig, expect_sig);
    free(msg);
    free(pk);
    free(sk);
    free(sig);
    free(generated_sig);
    if (rc == 0) {
        printf("mldsa KAT passed\n");
    }
    return rc;
}
