#include "pqc_module.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void print_usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s help\n", prog);
    printf("  %s info\n", prog);
    printf("  %s keygen --pub <file> --sec <file>\n", prog);
    printf("  %s encaps --pub <file> --ct <file> --ss <file>\n", prog);
    printf("  %s decaps --sec <file> --ct <file> --ss <file>\n", prog);
    printf("  %s benchmark --iterations <N>\n", prog);
    printf("Options:\n");
    printf("  --alg <dummy|mlkem-ref>\n");
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

static int handle_info(void) {
    size_t pk, sk, ct, ss;
    pqc_get_sizes(&pk, &sk, &ct, &ss);
    printf("Algorithm: %s\n", pqc_get_algorithm_name());
    printf("public_key_size=%zu\n", pk);
    printf("secret_key_size=%zu\n", sk);
    printf("ciphertext_size=%zu\n", ct);
    printf("shared_secret_size=%zu\n", ss);
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

static int handle_benchmark(int argc, char **argv) {
    const char *iter_arg = arg_value(argc, argv, "--iterations");
    int iterations = iter_arg ? atoi(iter_arg) : 1000;
    int i;
    clock_t start, end;
    double keygen_ms;
    double encaps_ms;
    double decaps_ms;
    size_t pk, sk, ct, ss;
    uint8_t *pub = NULL;
    uint8_t *sec = NULL;
    uint8_t *ciphertext = NULL;
    uint8_t *shared_secret = NULL;
    pqc_status_t st;

    if (iterations <= 0) {
        fprintf(stderr, "iterations must be > 0\n");
        return 1;
    }

    pqc_get_sizes(&pk, &sk, &ct, &ss);
    pub = (uint8_t *)malloc(pk);
    sec = (uint8_t *)malloc(sk);
    ciphertext = (uint8_t *)malloc(ct);
    shared_secret = (uint8_t *)malloc(ss);
    if (pub == NULL || sec == NULL || ciphertext == NULL || shared_secret == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        free(pub);
        free(sec);
        free(ciphertext);
        free(shared_secret);
        return 1;
    }

    start = clock();
    for (i = 0; i < iterations; ++i) {
        st = pqc_kem_keypair(pub, pk, sec, sk);
        if (st != PQC_OK) {
            fprintf(stderr, "benchmark keygen failed: %s\n", pqc_status_to_string(st));
            return 1;
        }
    }
    end = clock();
    keygen_ms = ((double)(end - start) * 1000.0 / CLOCKS_PER_SEC) / iterations;

    st = pqc_kem_keypair(pub, pk, sec, sk);
    if (st != PQC_OK) {
        fprintf(stderr, "benchmark setup failed: %s\n", pqc_status_to_string(st));
        return 1;
    }

    start = clock();
    for (i = 0; i < iterations; ++i) {
        st = pqc_kem_encaps(ciphertext, ct, shared_secret, ss, pub, pk);
        if (st != PQC_OK) {
            fprintf(stderr, "benchmark encaps failed: %s\n", pqc_status_to_string(st));
            return 1;
        }
    }
    end = clock();
    encaps_ms = ((double)(end - start) * 1000.0 / CLOCKS_PER_SEC) / iterations;

    st = pqc_kem_encaps(ciphertext, ct, shared_secret, ss, pub, pk);
    if (st != PQC_OK) {
        fprintf(stderr, "benchmark setup failed: %s\n", pqc_status_to_string(st));
        return 1;
    }

    start = clock();
    for (i = 0; i < iterations; ++i) {
        st = pqc_kem_decaps(shared_secret, ss, ciphertext, ct, sec, sk);
        if (st != PQC_OK) {
            fprintf(stderr, "benchmark decaps failed: %s\n", pqc_status_to_string(st));
            return 1;
        }
    }
    end = clock();
    decaps_ms = ((double)(end - start) * 1000.0 / CLOCKS_PER_SEC) / iterations;

    printf("benchmark iterations=%d\n", iterations);
    printf("algorithm=%s\n", pqc_get_algorithm_name());
    printf("avg_keygen_ms=%.3f\n", keygen_ms);
    printf("avg_encaps_ms=%.3f\n", encaps_ms);
    printf("avg_decaps_ms=%.3f\n", decaps_ms);
    printf("note=development_benchmark_not_for_production\n");

    secure_memzero(sec, sk);
    secure_memzero(shared_secret, ss);
    free(pub);
    free(sec);
    free(ciphertext);
    free(shared_secret);
    return 0;
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
    if (strcmp(cmd, "encaps") == 0) {
        return handle_encaps(argc - 1, argv + 1);
    }
    if (strcmp(cmd, "decaps") == 0) {
        return handle_decaps(argc - 1, argv + 1);
    }
    if (strcmp(cmd, "benchmark") == 0) {
        return handle_benchmark(argc - 1, argv + 1);
    }

    fprintf(stderr, "unknown command: %s\n", cmd);
    print_usage(argv[0]);
    return 1;
}
