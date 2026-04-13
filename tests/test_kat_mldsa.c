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
    char sig_path[512];
    uint8_t *msg = NULL;
    uint8_t *pk = NULL;
    uint8_t *sig = NULL;
    size_t msg_len = 0;
    size_t pk_len = 0;
    size_t sig_len = 0;
    size_t expect_pk = 0;
    size_t expect_sk_unused = 0;
    size_t expect_sig = 0;
    pqc_status_t st;

#ifndef TESTS_DATA_DIR
#define TESTS_DATA_DIR "tests/data"
#endif

    (void)snprintf(msg_path, sizeof(msg_path), "%s/kat_mldsa_msg.txt", TESTS_DATA_DIR);
    (void)snprintf(pk_path, sizeof(pk_path), "%s/kat_mldsa_pub.key", TESTS_DATA_DIR);
    (void)snprintf(sig_path, sizeof(sig_path), "%s/kat_mldsa.sig", TESTS_DATA_DIR);

    if (pqc_set_signature_algorithm(PQC_SIG_ALG_ML_DSA_65_REF) != PQC_OK) {
        printf("mldsa ref backend unavailable, skip\n");
        return 0;
    }

    pqc_sig_get_sizes(&expect_pk, &expect_sk_unused, &expect_sig);
    if (!read_file(msg_path, &msg, &msg_len) ||
        !read_file(pk_path, &pk, &pk_len) ||
        !read_file(sig_path, &sig, &sig_len)) {
        fprintf(stderr, "failed to read KAT files\n");
        return 1;
    }

    if (pk_len != expect_pk || sig_len != expect_sig) {
        fprintf(stderr, "KAT size mismatch\n");
        return 1;
    }

    st = pqc_sig_verify(sig, sig_len, msg, msg_len, pk, pk_len);
    if (st != PQC_OK) {
        fprintf(stderr, "KAT verify failed\n");
        return 1;
    }

    sig[0] ^= 0x01;
    st = pqc_sig_verify(sig, sig_len, msg, msg_len, pk, pk_len);
    if (st != PQC_ERR_VERIFY_FAILED) {
        fprintf(stderr, "tampered KAT signature should fail\n");
        return 1;
    }

    free(msg);
    free(pk);
    free(sig);
    printf("mldsa KAT passed\n");
    return 0;
}
