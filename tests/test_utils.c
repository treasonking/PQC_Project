#include "pqc_module.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t secret[32];
    size_t i;

    memset(secret, 0xA5, sizeof(secret));
    secure_memzero(secret, sizeof(secret));

    for (i = 0; i < sizeof(secret); ++i) {
        if (secret[i] != 0) {
            fprintf(stderr, "secure_memzero left non-zero data at offset %zu\n", i);
            return 1;
        }
    }

    secure_memzero(NULL, 0);
    secure_memzero(secret, 0);

    printf("utils tests passed\n");
    return 0;
}
