#define __STDC_WANT_LIB_EXT1__ 1

#include "pqc_module.h"

#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(_MSC_VER)
#include <intrin.h>
#endif

void secure_memzero(void *ptr, size_t len) {
    if (ptr == NULL || len == 0) {
        return;
    }

    /*
     * Use a guaranteed zeroing primitive when the platform exposes one. The
     * fallback keeps the write visible to the compiler with a memory barrier.
     * This reduces optimization-removal risk, but platform-level guarantees
     * still require toolchain/runtime review for production use.
     */
#if defined(_WIN32)
    SecureZeroMemory(ptr, len);
#elif defined(__STDC_LIB_EXT1__)
    (void)memset_s(ptr, len, 0, len);
#else
    memset(ptr, 0, len);
#if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__("" : : "r"(ptr) : "memory");
#elif defined(_MSC_VER)
    _ReadWriteBarrier();
#else
    {
        volatile uint8_t *p = (volatile uint8_t *)ptr;
        size_t i;
        for (i = 0; i < len; ++i) {
            p[i] = 0;
        }
    }
#endif
#endif
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
