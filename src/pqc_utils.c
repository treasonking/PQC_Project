#include "pqc_module.h"

void secure_memzero(void *ptr, size_t len) {
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    size_t i;
    if (ptr == NULL) {
        return;
    }
    for (i = 0; i < len; ++i) {
        p[i] = 0;
    }
}

const char *pqc_status_to_string(pqc_status_t status) {
    switch (status) {
        case PQC_OK:
            return "PQC_OK";
        case PQC_ERR_INVALID_ARG:
            return "PQC_ERR_INVALID_ARG";
        case PQC_ERR_BUFFER_TOO_SMALL:
            return "PQC_ERR_BUFFER_TOO_SMALL";
        case PQC_ERR_KEYGEN_FAILED:
            return "PQC_ERR_KEYGEN_FAILED";
        case PQC_ERR_SIGN_FAILED:
            return "PQC_ERR_SIGN_FAILED";
        case PQC_ERR_ENCAP_FAILED:
            return "PQC_ERR_ENCAP_FAILED";
        case PQC_ERR_DECAP_FAILED:
            return "PQC_ERR_DECAP_FAILED";
        case PQC_ERR_VERIFY_FAILED:
            return "PQC_ERR_VERIFY_FAILED";
        case PQC_ERR_INTERNAL:
            return "PQC_ERR_INTERNAL";
        default:
            return "PQC_ERR_UNKNOWN";
    }
}
