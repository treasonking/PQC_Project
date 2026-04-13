#include "pqc_module.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int has_csv_extension(const char *path) {
    const char *dot;
    if (path == NULL) {
        return 0;
    }
    dot = strrchr(path, '.');
    return dot != NULL && strcmp(dot, ".csv") == 0;
}

static int write_benchmark_output(const char *out_path,
                                  const char *kem_algorithm,
                                  const char *sig_algorithm,
                                  int iterations,
                                  size_t pk,
                                  size_t sk,
                                  size_t ct,
                                  size_t ss,
                                  size_t sig_pk,
                                  size_t sig_sk,
                                  size_t sig_size,
                                  double keygen_ms,
                                  double encaps_ms,
                                  double decaps_ms,
                                  double sig_keygen_ms,
                                  double sign_ms,
                                  double verify_ms) {
    FILE *fp;
    time_t now;
    struct tm *tm_info;
    char timestamp[64];

    if (out_path == NULL) {
        return 1;
    }

    now = time(NULL);
    tm_info = localtime(&now);
    if (tm_info == NULL) {
        return 0;
    }
    if (strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info) == 0) {
        return 0;
    }

    if (has_csv_extension(out_path)) {
        int write_header = 0;
        fp = fopen(out_path, "rb");
        if (fp == NULL) {
            write_header = 1;
        } else {
            if (fseek(fp, 0, SEEK_END) != 0 || ftell(fp) <= 0) {
                write_header = 1;
            }
            fclose(fp);
        }

        fp = fopen(out_path, "ab");
        if (fp == NULL) {
            return 0;
        }
        if (write_header) {
            fprintf(fp, "timestamp,kem_algorithm,sig_algorithm,iterations,public_key_size,secret_key_size,ciphertext_size,shared_secret_size,sig_public_key_size,sig_secret_key_size,signature_size,avg_keygen_ms,avg_encaps_ms,avg_decaps_ms,avg_sig_keygen_ms,avg_sign_ms,avg_verify_ms\n");
        }
        fprintf(fp, "%s,%s,%s,%d,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
                timestamp, kem_algorithm, sig_algorithm, iterations, pk, sk, ct, ss, sig_pk, sig_sk, sig_size, keygen_ms, encaps_ms, decaps_ms, sig_keygen_ms, sign_ms, verify_ms);
        fclose(fp);
        return 1;
    }

    fp = fopen(out_path, "wb");
    if (fp == NULL) {
        return 0;
    }
    fprintf(fp, "benchmark iterations=%d\n", iterations);
    fprintf(fp, "kem_algorithm=%s\n", kem_algorithm);
    fprintf(fp, "sig_algorithm=%s\n", sig_algorithm);
    fprintf(fp, "public_key_size=%zu\n", pk);
    fprintf(fp, "secret_key_size=%zu\n", sk);
    fprintf(fp, "ciphertext_size=%zu\n", ct);
    fprintf(fp, "shared_secret_size=%zu\n", ss);
    fprintf(fp, "sig_public_key_size=%zu\n", sig_pk);
    fprintf(fp, "sig_secret_key_size=%zu\n", sig_sk);
    fprintf(fp, "signature_size=%zu\n", sig_size);
    fprintf(fp, "avg_keygen_ms=%.6f\n", keygen_ms);
    fprintf(fp, "avg_encaps_ms=%.6f\n", encaps_ms);
    fprintf(fp, "avg_decaps_ms=%.6f\n", decaps_ms);
    fprintf(fp, "avg_sig_keygen_ms=%.6f\n", sig_keygen_ms);
    fprintf(fp, "avg_sign_ms=%.6f\n", sign_ms);
    fprintf(fp, "avg_verify_ms=%.6f\n", verify_ms);
    fclose(fp);
    return 1;
}

static void print_usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s help\n", prog);
    printf("  %s info\n", prog);
    printf("  %s keygen --pub <file> --sec <file>\n", prog);
    printf("  %s sig-keygen --pub <file> --sec <file>\n", prog);
    printf("  %s encaps --pub <file> --ct <file> --ss <file>\n", prog);
    printf("  %s decaps --sec <file> --ct <file> --ss <file>\n", prog);
    printf("  %s sign --sec <file> --msg <file> --sig <file>\n", prog);
    printf("  %s verify --pub <file> --msg <file> --sig <file>\n", prog);
    printf("  %s benchmark --iterations <N> [--out <result.txt|result.csv>]\n", prog);
    printf("Options:\n");
    printf("  --alg <dummy|mlkem-ref>\n");
    printf("  --sig-alg <dummy-dsa|mldsa-ref>\n");
}

static const char *arg_value(int argc, char **argv, const char *name) {
    int i;
    for (i = 0; i < argc - 1; ++i) {
        if (strcmp(argv[i], name) == 0) {
            return argv[i + 1];
        }
    }
    return NULL;
}

static int maybe_set_algorithm_from_args(int argc, char **argv) {
    const char *alg = arg_value(argc, argv, "--alg");
    pqc_status_t st;

    if (alg == NULL) {
        return 0;
    }
    if (strcmp(alg, "dummy") == 0) {
        st = pqc_set_algorithm(PQC_ALG_ML_KEM_768_DUMMY);
    } else if (strcmp(alg, "mlkem-ref") == 0) {
        st = pqc_set_algorithm(PQC_ALG_ML_KEM_768_REF);
    } else {
        fprintf(stderr, "unsupported --alg value: %s\n", alg);
        return 1;
    }
    if (st != PQC_OK) {
        fprintf(stderr, "failed to set algorithm: %s\n", pqc_status_to_string(st));
        return 1;
    }
    return 0;
}

static int maybe_set_signature_algorithm_from_args(int argc, char **argv) {
    const char *alg = arg_value(argc, argv, "--sig-alg");
    pqc_status_t st;

    if (alg == NULL) {
        return 0;
    }
    if (strcmp(alg, "dummy-dsa") == 0) {
        st = pqc_set_signature_algorithm(PQC_SIG_ALG_ML_DSA_65_DUMMY);
    } else if (strcmp(alg, "mldsa-ref") == 0) {
        st = pqc_set_signature_algorithm(PQC_SIG_ALG_ML_DSA_65_REF);
    } else {
        fprintf(stderr, "unsupported --sig-alg value: %s\n", alg);
        return 1;
    }
    if (st != PQC_OK) {
        fprintf(stderr, "failed to set signature algorithm: %s\n", pqc_status_to_string(st));
        return 1;
    }
    return 0;
}

static int write_binary_file(const char *path, const uint8_t *data, size_t len) {
    FILE *fp;
    if (path == NULL || data == NULL) {
        return 0;
    }
    fp = fopen(path, "wb");
    if (fp == NULL) {
        return 0;
    }
    if (fwrite(data, 1, len, fp) != len) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int read_binary_file(const char *path, uint8_t *data, size_t len) {
    FILE *fp;
    size_t read_size;
    if (path == NULL || data == NULL) {
        return 0;
    }
    fp = fopen(path, "rb");
    if (fp == NULL) {
        return 0;
    }
    read_size = fread(data, 1, len, fp);
    fclose(fp);
    return read_size == len;
}

static int read_binary_file_dynamic(const char *path, uint8_t **data, size_t *len) {
    FILE *fp;
    long sz;
    uint8_t *buf;
    size_t read_size;
    if (path == NULL || data == NULL || len == NULL) {
        return 0;
    }
    *data = NULL;
    *len = 0;
    fp = fopen(path, "rb");
    if (fp == NULL) {
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
    read_size = fread(buf, 1, (size_t)sz, fp);
    fclose(fp);
    if (read_size != (size_t)sz) {
        free(buf);
        return 0;
    }
    *data = buf;
    *len = (size_t)sz;
    return 1;
}

static int handle_info(void) {
    size_t pk, sk, ct, ss;
    size_t spk, ssk, sig;
    pqc_get_sizes(&pk, &sk, &ct, &ss);
    pqc_sig_get_sizes(&spk, &ssk, &sig);
    printf("Algorithm: %s\n", pqc_get_algorithm_name());
    printf("public_key_size=%zu\n", pk);
    printf("secret_key_size=%zu\n", sk);
    printf("ciphertext_size=%zu\n", ct);
    printf("shared_secret_size=%zu\n", ss);
    printf("signature_algorithm=%s\n", pqc_get_signature_algorithm_name());
    printf("sig_public_key_size=%zu\n", spk);
    printf("sig_secret_key_size=%zu\n", ssk);
    printf("signature_size=%zu\n", sig);
    return 0;
}

static int handle_keygen(int argc, char **argv) {
    const char *pub_path = arg_value(argc, argv, "--pub");
    const char *sec_path = arg_value(argc, argv, "--sec");
    size_t pk, sk;
    uint8_t *pub = NULL;
    uint8_t *sec = NULL;
    pqc_status_t st;
    int ok = 1;

    if (pub_path == NULL || sec_path == NULL) {
        fprintf(stderr, "keygen requires --pub and --sec\n");
        return 1;
    }

    pqc_get_sizes(&pk, &sk, NULL, NULL);
    pub = (uint8_t *)malloc(pk);
    sec = (uint8_t *)malloc(sk);
    if (pub == NULL || sec == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        free(pub);
        free(sec);
        return 1;
    }

    st = pqc_kem_keypair(pub, pk, sec, sk);
    if (st != PQC_OK) {
        fprintf(stderr, "keygen failed: %s\n", pqc_status_to_string(st));
        ok = 0;
    } else if (!write_binary_file(pub_path, pub, pk) || !write_binary_file(sec_path, sec, sk)) {
        fprintf(stderr, "failed to write key files\n");
        ok = 0;
    } else {
        printf("keygen ok\n");
    }

    secure_memzero(sec, sk);
    free(pub);
    free(sec);
    return ok ? 0 : 1;
}

static int handle_encaps(int argc, char **argv) {
    const char *pub_path = arg_value(argc, argv, "--pub");
    const char *ct_path = arg_value(argc, argv, "--ct");
    const char *ss_path = arg_value(argc, argv, "--ss");
    size_t pk, ct, ss;
    uint8_t *pub = NULL;
    uint8_t *ciphertext = NULL;
    uint8_t *shared_secret = NULL;
    pqc_status_t st;
    int ok = 1;

    if (pub_path == NULL || ct_path == NULL || ss_path == NULL) {
        fprintf(stderr, "encaps requires --pub, --ct and --ss\n");
        return 1;
    }

    pqc_get_sizes(&pk, NULL, &ct, &ss);
    pub = (uint8_t *)malloc(pk);
    ciphertext = (uint8_t *)malloc(ct);
    shared_secret = (uint8_t *)malloc(ss);
    if (pub == NULL || ciphertext == NULL || shared_secret == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        ok = 0;
        goto done;
    }

    if (!read_binary_file(pub_path, pub, pk)) {
        fprintf(stderr, "failed to read public key file\n");
        ok = 0;
        goto done;
    }

    st = pqc_kem_encaps(ciphertext, ct, shared_secret, ss, pub, pk);
    if (st != PQC_OK) {
        fprintf(stderr, "encaps failed: %s\n", pqc_status_to_string(st));
        ok = 0;
        goto done;
    }
    if (!write_binary_file(ct_path, ciphertext, ct) || !write_binary_file(ss_path, shared_secret, ss)) {
        fprintf(stderr, "failed to write output files\n");
        ok = 0;
        goto done;
    }

    printf("encaps ok\n");

done:
    secure_memzero(shared_secret, ss);
    free(pub);
    free(ciphertext);
    free(shared_secret);
    return ok ? 0 : 1;
}

static int handle_sig_keygen(int argc, char **argv) {
    const char *pub_path = arg_value(argc, argv, "--pub");
    const char *sec_path = arg_value(argc, argv, "--sec");
    size_t pk, sk, sig_size_unused;
    uint8_t *pub = NULL;
    uint8_t *sec = NULL;
    pqc_status_t st;
    int ok = 1;
    (void)sig_size_unused;

    if (pub_path == NULL || sec_path == NULL) {
        fprintf(stderr, "sig-keygen requires --pub and --sec\n");
        return 1;
    }

    pqc_sig_get_sizes(&pk, &sk, &sig_size_unused);
    pub = (uint8_t *)malloc(pk);
    sec = (uint8_t *)malloc(sk);
    if (pub == NULL || sec == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        free(pub);
        free(sec);
        return 1;
    }

    st = pqc_sig_keypair(pub, pk, sec, sk);
    if (st != PQC_OK) {
        fprintf(stderr, "sig-keygen failed: %s\n", pqc_status_to_string(st));
        ok = 0;
    } else if (!write_binary_file(pub_path, pub, pk) || !write_binary_file(sec_path, sec, sk)) {
        fprintf(stderr, "failed to write signature key files\n");
        ok = 0;
    } else {
        printf("sig-keygen ok\n");
    }

    secure_memzero(sec, sk);
    free(pub);
    free(sec);
    return ok ? 0 : 1;
}

static int handle_decaps(int argc, char **argv) {
    const char *sec_path = arg_value(argc, argv, "--sec");
    const char *ct_path = arg_value(argc, argv, "--ct");
    const char *ss_path = arg_value(argc, argv, "--ss");
    size_t sk, ct, ss;
    uint8_t *sec = NULL;
    uint8_t *ciphertext = NULL;
    uint8_t *shared_secret = NULL;
    pqc_status_t st;
    int ok = 1;

    if (sec_path == NULL || ct_path == NULL || ss_path == NULL) {
        fprintf(stderr, "decaps requires --sec, --ct and --ss\n");
        return 1;
    }

    pqc_get_sizes(NULL, &sk, &ct, &ss);
    sec = (uint8_t *)malloc(sk);
    ciphertext = (uint8_t *)malloc(ct);
    shared_secret = (uint8_t *)malloc(ss);
    if (sec == NULL || ciphertext == NULL || shared_secret == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        ok = 0;
        goto done;
    }

    if (!read_binary_file(sec_path, sec, sk) || !read_binary_file(ct_path, ciphertext, ct)) {
        fprintf(stderr, "failed to read input files\n");
        ok = 0;
        goto done;
    }

    st = pqc_kem_decaps(shared_secret, ss, ciphertext, ct, sec, sk);
    if (st != PQC_OK) {
        fprintf(stderr, "decaps failed: %s\n", pqc_status_to_string(st));
        ok = 0;
        goto done;
    }
    if (!write_binary_file(ss_path, shared_secret, ss)) {
        fprintf(stderr, "failed to write shared secret file\n");
        ok = 0;
        goto done;
    }

    printf("decaps ok\n");

done:
    secure_memzero(sec, sk);
    secure_memzero(shared_secret, ss);
    free(sec);
    free(ciphertext);
    free(shared_secret);
    return ok ? 0 : 1;
}

static int handle_sign(int argc, char **argv) {
    const char *sec_path = arg_value(argc, argv, "--sec");
    const char *msg_path = arg_value(argc, argv, "--msg");
    const char *sig_path = arg_value(argc, argv, "--sig");
    size_t sk, sig_len, pk_unused;
    uint8_t *sec = NULL;
    uint8_t *msg = NULL;
    uint8_t *sig = NULL;
    size_t msg_len = 0;
    pqc_status_t st;
    int ok = 1;
    (void)pk_unused;

    if (sec_path == NULL || msg_path == NULL || sig_path == NULL) {
        fprintf(stderr, "sign requires --sec, --msg and --sig\n");
        return 1;
    }

    pqc_sig_get_sizes(&pk_unused, &sk, &sig_len);
    sec = (uint8_t *)malloc(sk);
    sig = (uint8_t *)malloc(sig_len);
    if (sec == NULL || sig == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        ok = 0;
        goto done;
    }
    if (!read_binary_file(sec_path, sec, sk)) {
        fprintf(stderr, "failed to read signature secret key file\n");
        ok = 0;
        goto done;
    }
    if (!read_binary_file_dynamic(msg_path, &msg, &msg_len)) {
        fprintf(stderr, "failed to read message file\n");
        ok = 0;
        goto done;
    }

    st = pqc_sig_sign(sig, sig_len, msg, msg_len, sec, sk);
    if (st != PQC_OK) {
        fprintf(stderr, "sign failed: %s\n", pqc_status_to_string(st));
        ok = 0;
        goto done;
    }
    if (!write_binary_file(sig_path, sig, sig_len)) {
        fprintf(stderr, "failed to write signature file\n");
        ok = 0;
        goto done;
    }
    printf("sign ok\n");

done:
    secure_memzero(sec, sk);
    secure_memzero(sig, sig_len);
    free(sec);
    free(msg);
    free(sig);
    return ok ? 0 : 1;
}

static int handle_verify(int argc, char **argv) {
    const char *pub_path = arg_value(argc, argv, "--pub");
    const char *msg_path = arg_value(argc, argv, "--msg");
    const char *sig_path = arg_value(argc, argv, "--sig");
    size_t pk, sk_unused, sig_len;
    uint8_t *pub = NULL;
    uint8_t *msg = NULL;
    uint8_t *sig = NULL;
    size_t msg_len = 0;
    pqc_status_t st;
    int ok = 1;
    (void)sk_unused;

    if (pub_path == NULL || msg_path == NULL || sig_path == NULL) {
        fprintf(stderr, "verify requires --pub, --msg and --sig\n");
        return 1;
    }

    pqc_sig_get_sizes(&pk, &sk_unused, &sig_len);
    pub = (uint8_t *)malloc(pk);
    sig = (uint8_t *)malloc(sig_len);
    if (pub == NULL || sig == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        ok = 0;
        goto done;
    }
    if (!read_binary_file(pub_path, pub, pk)) {
        fprintf(stderr, "failed to read signature public key file\n");
        ok = 0;
        goto done;
    }
    if (!read_binary_file(sig_path, sig, sig_len)) {
        fprintf(stderr, "failed to read signature file\n");
        ok = 0;
        goto done;
    }
    if (!read_binary_file_dynamic(msg_path, &msg, &msg_len)) {
        fprintf(stderr, "failed to read message file\n");
        ok = 0;
        goto done;
    }

    st = pqc_sig_verify(sig, sig_len, msg, msg_len, pub, pk);
    if (st != PQC_OK) {
        fprintf(stderr, "verify failed: %s\n", pqc_status_to_string(st));
        ok = 0;
        goto done;
    }

    printf("verify ok\n");

done:
    free(pub);
    free(msg);
    free(sig);
    return ok ? 0 : 1;
}

static int handle_benchmark(int argc, char **argv) {
    const char *iter_arg = arg_value(argc, argv, "--iterations");
    const char *out_path = arg_value(argc, argv, "--out");
    int iterations = iter_arg ? atoi(iter_arg) : 1000;
    int i;
    clock_t start, end;
    double keygen_ms;
    double encaps_ms;
    double decaps_ms;
    double sig_keygen_ms;
    double sign_ms;
    double verify_ms;
    size_t pk, sk, ct, ss;
    size_t sig_pk, sig_sk, sig_len;
    uint8_t *pub = NULL;
    uint8_t *sec = NULL;
    uint8_t *ciphertext = NULL;
    uint8_t *shared_secret = NULL;
    uint8_t *sig_pub = NULL;
    uint8_t *sig_sec = NULL;
    uint8_t *signature = NULL;
    const uint8_t benchmark_msg[] = "pqc benchmark signing message";
    pqc_status_t st;
    int rc = 0;

    if (iterations <= 0) {
        fprintf(stderr, "iterations must be > 0\n");
        return 1;
    }

    pqc_get_sizes(&pk, &sk, &ct, &ss);
    pqc_sig_get_sizes(&sig_pk, &sig_sk, &sig_len);
    pub = (uint8_t *)malloc(pk);
    sec = (uint8_t *)malloc(sk);
    ciphertext = (uint8_t *)malloc(ct);
    shared_secret = (uint8_t *)malloc(ss);
    sig_pub = (uint8_t *)malloc(sig_pk);
    sig_sec = (uint8_t *)malloc(sig_sk);
    signature = (uint8_t *)malloc(sig_len);
    if (pub == NULL || sec == NULL || ciphertext == NULL || shared_secret == NULL ||
        sig_pub == NULL || sig_sec == NULL || signature == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        rc = 1;
        goto done;
    }

    start = clock();
    for (i = 0; i < iterations; ++i) {
        st = pqc_kem_keypair(pub, pk, sec, sk);
        if (st != PQC_OK) {
            fprintf(stderr, "benchmark keygen failed: %s\n", pqc_status_to_string(st));
            rc = 1;
            goto done;
        }
    }
    end = clock();
    keygen_ms = ((double)(end - start) * 1000.0 / CLOCKS_PER_SEC) / iterations;

    st = pqc_kem_keypair(pub, pk, sec, sk);
    if (st != PQC_OK) {
        fprintf(stderr, "benchmark setup failed: %s\n", pqc_status_to_string(st));
        rc = 1;
        goto done;
    }

    start = clock();
    for (i = 0; i < iterations; ++i) {
        st = pqc_kem_encaps(ciphertext, ct, shared_secret, ss, pub, pk);
        if (st != PQC_OK) {
            fprintf(stderr, "benchmark encaps failed: %s\n", pqc_status_to_string(st));
            rc = 1;
            goto done;
        }
    }
    end = clock();
    encaps_ms = ((double)(end - start) * 1000.0 / CLOCKS_PER_SEC) / iterations;

    st = pqc_kem_encaps(ciphertext, ct, shared_secret, ss, pub, pk);
    if (st != PQC_OK) {
        fprintf(stderr, "benchmark setup failed: %s\n", pqc_status_to_string(st));
        rc = 1;
        goto done;
    }

    start = clock();
    for (i = 0; i < iterations; ++i) {
        st = pqc_kem_decaps(shared_secret, ss, ciphertext, ct, sec, sk);
        if (st != PQC_OK) {
            fprintf(stderr, "benchmark decaps failed: %s\n", pqc_status_to_string(st));
            rc = 1;
            goto done;
        }
    }
    end = clock();
    decaps_ms = ((double)(end - start) * 1000.0 / CLOCKS_PER_SEC) / iterations;

    start = clock();
    for (i = 0; i < iterations; ++i) {
        st = pqc_sig_keypair(sig_pub, sig_pk, sig_sec, sig_sk);
        if (st != PQC_OK) {
            fprintf(stderr, "benchmark sig keygen failed: %s\n", pqc_status_to_string(st));
            rc = 1;
            goto done;
        }
    }
    end = clock();
    sig_keygen_ms = ((double)(end - start) * 1000.0 / CLOCKS_PER_SEC) / iterations;

    st = pqc_sig_keypair(sig_pub, sig_pk, sig_sec, sig_sk);
    if (st != PQC_OK) {
        fprintf(stderr, "benchmark signature setup failed: %s\n", pqc_status_to_string(st));
        rc = 1;
        goto done;
    }

    start = clock();
    for (i = 0; i < iterations; ++i) {
        st = pqc_sig_sign(signature,
                          sig_len,
                          benchmark_msg,
                          sizeof(benchmark_msg) - 1,
                          sig_sec,
                          sig_sk);
        if (st != PQC_OK) {
            fprintf(stderr, "benchmark sign failed: %s\n", pqc_status_to_string(st));
            rc = 1;
            goto done;
        }
    }
    end = clock();
    sign_ms = ((double)(end - start) * 1000.0 / CLOCKS_PER_SEC) / iterations;

    st = pqc_sig_sign(signature,
                      sig_len,
                      benchmark_msg,
                      sizeof(benchmark_msg) - 1,
                      sig_sec,
                      sig_sk);
    if (st != PQC_OK) {
        fprintf(stderr, "benchmark signature setup failed: %s\n", pqc_status_to_string(st));
        rc = 1;
        goto done;
    }

    start = clock();
    for (i = 0; i < iterations; ++i) {
        st = pqc_sig_verify(signature,
                            sig_len,
                            benchmark_msg,
                            sizeof(benchmark_msg) - 1,
                            sig_pub,
                            sig_pk);
        if (st != PQC_OK) {
            fprintf(stderr, "benchmark verify failed: %s\n", pqc_status_to_string(st));
            rc = 1;
            goto done;
        }
    }
    end = clock();
    verify_ms = ((double)(end - start) * 1000.0 / CLOCKS_PER_SEC) / iterations;

    printf("benchmark iterations=%d\n", iterations);
    printf("kem_algorithm=%s\n", pqc_get_algorithm_name());
    printf("sig_algorithm=%s\n", pqc_get_signature_algorithm_name());
    printf("public_key_size=%zu\n", pk);
    printf("secret_key_size=%zu\n", sk);
    printf("ciphertext_size=%zu\n", ct);
    printf("shared_secret_size=%zu\n", ss);
    printf("sig_public_key_size=%zu\n", sig_pk);
    printf("sig_secret_key_size=%zu\n", sig_sk);
    printf("signature_size=%zu\n", sig_len);
    printf("avg_keygen_ms=%.3f\n", keygen_ms);
    printf("avg_encaps_ms=%.3f\n", encaps_ms);
    printf("avg_decaps_ms=%.3f\n", decaps_ms);
    printf("avg_sig_keygen_ms=%.3f\n", sig_keygen_ms);
    printf("avg_sign_ms=%.3f\n", sign_ms);
    printf("avg_verify_ms=%.3f\n", verify_ms);
    printf("note=development_benchmark_not_for_production\n");

    if (out_path != NULL) {
        if (!write_benchmark_output(out_path,
                                    pqc_get_algorithm_name(),
                                    pqc_get_signature_algorithm_name(),
                                    iterations,
                                    pk,
                                    sk,
                                    ct,
                                    ss,
                                    sig_pk,
                                    sig_sk,
                                    sig_len,
                                    keygen_ms,
                                    encaps_ms,
                                    decaps_ms,
                                    sig_keygen_ms,
                                    sign_ms,
                                    verify_ms)) {
            fprintf(stderr, "failed to write benchmark output file: %s\n", out_path);
            rc = 1;
            goto done;
        }
        printf("saved=%s\n", out_path);
    }

done:
    secure_memzero(sec, sk);
    secure_memzero(shared_secret, ss);
    secure_memzero(sig_sec, sig_sk);
    secure_memzero(signature, sig_len);
    free(pub);
    free(sec);
    free(ciphertext);
    free(shared_secret);
    free(sig_pub);
    free(sig_sec);
    free(signature);
    return rc;
}

int main(int argc, char **argv) {
    const char *cmd;
    if (argc < 2) {
        print_usage(argv[0]);
        return 0;
    }

    cmd = argv[1];
    if (maybe_set_algorithm_from_args(argc, argv) != 0) {
        return 1;
    }
    if (maybe_set_signature_algorithm_from_args(argc, argv) != 0) {
        return 1;
    }
    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    if (strcmp(cmd, "info") == 0) {
        return handle_info();
    }
    if (strcmp(cmd, "keygen") == 0) {
        return handle_keygen(argc - 1, argv + 1);
    }
    if (strcmp(cmd, "sig-keygen") == 0) {
        return handle_sig_keygen(argc - 1, argv + 1);
    }
    if (strcmp(cmd, "encaps") == 0) {
        return handle_encaps(argc - 1, argv + 1);
    }
    if (strcmp(cmd, "decaps") == 0) {
        return handle_decaps(argc - 1, argv + 1);
    }
    if (strcmp(cmd, "sign") == 0) {
        return handle_sign(argc - 1, argv + 1);
    }
    if (strcmp(cmd, "verify") == 0) {
        return handle_verify(argc - 1, argv + 1);
    }
    if (strcmp(cmd, "benchmark") == 0) {
        return handle_benchmark(argc - 1, argv + 1);
    }

    fprintf(stderr, "unknown command: %s\n", cmd);
    print_usage(argv[0]);
    return 1;
}
