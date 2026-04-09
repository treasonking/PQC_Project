#include "pqc_module.h"

#include <stdio.h>

int main(void) {
    size_t pk, sk, ct, ss;
    pqc_get_sizes(&pk, &sk, &ct, &ss);

    printf("algorithm=%s\n", pqc_get_algorithm_name());
    printf("public_key_size=%zu\n", pk);
    printf("secret_key_size=%zu\n", sk);
    printf("ciphertext_size=%zu\n", ct);
    printf("shared_secret_size=%zu\n", ss);
    printf("note=use pqc_cli benchmark for timing in current phase\n");
    return 0;
}
