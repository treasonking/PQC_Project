cmake_minimum_required(VERSION 3.16)
project(pqc_crypto_module_lab VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(PQC_MODULE_SOURCES
    src/pqc_module.c
    src/pqc_kem.c
    src/pqc_sig.c
    src/pqc_utils.c
)

set(PQCLEAN_MLKEM768_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/mlkem_pqclean/clean")
set(PQCLEAN_MLDSA65_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/mldsa_pqclean/clean")
set(PQCLEAN_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/mlkem_pqclean/common")

if(EXISTS "${PQCLEAN_MLKEM768_DIR}/api.h" AND EXISTS "${PQCLEAN_COMMON_DIR}/fips202.c")
    message(STATUS "Enabling PQClean ML-KEM-768 reference backend")
    list(APPEND PQC_MODULE_SOURCES
        "${PQCLEAN_MLKEM768_DIR}/cbd.c"
        "${PQCLEAN_MLKEM768_DIR}/indcpa.c"
        "${PQCLEAN_MLKEM768_DIR}/kem.c"
        "${PQCLEAN_MLKEM768_DIR}/ntt.c"
        "${PQCLEAN_MLKEM768_DIR}/poly.c"
        "${PQCLEAN_MLKEM768_DIR}/polyvec.c"
        "${PQCLEAN_MLKEM768_DIR}/reduce.c"
        "${PQCLEAN_MLKEM768_DIR}/symmetric-shake.c"
        "${PQCLEAN_MLKEM768_DIR}/verify.c"
        "${PQCLEAN_COMMON_DIR}/fips202.c"
        "${PQCLEAN_COMMON_DIR}/randombytes.c"
    )
    set(PQC_ENABLE_MLKEM_REF_VALUE 1)
else()
    message(STATUS "PQClean ML-KEM-768 not found; building with dummy backend only")
    set(PQC_ENABLE_MLKEM_REF_VALUE 0)
endif()

if(EXISTS "${PQCLEAN_MLDSA65_DIR}/api.h")
    message(STATUS "Enabling PQClean ML-DSA-65 reference backend")
    list(APPEND PQC_MODULE_SOURCES
        "${PQCLEAN_MLDSA65_DIR}/ntt.c"
        "${PQCLEAN_MLDSA65_DIR}/packing.c"
        "${PQCLEAN_MLDSA65_DIR}/poly.c"
        "${PQCLEAN_MLDSA65_DIR}/polyvec.c"
        "${PQCLEAN_MLDSA65_DIR}/reduce.c"
        "${PQCLEAN_MLDSA65_DIR}/rounding.c"
        "${PQCLEAN_MLDSA65_DIR}/sign.c"
        "${PQCLEAN_MLDSA65_DIR}/symmetric-shake.c"
    )
    set(PQC_ENABLE_MLDSA_REF_VALUE 1)
else()
    message(STATUS "PQClean ML-DSA-65 not found; building with dummy signature backend only")
    set(PQC_ENABLE_MLDSA_REF_VALUE 0)
endif()

add_library(pqc_module
    ${PQC_MODULE_SOURCES}
)

target_include_directories(pqc_module
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${PQCLEAN_MLKEM768_DIR}
        ${PQCLEAN_MLDSA65_DIR}
        ${PQCLEAN_COMMON_DIR}
)

target_compile_definitions(pqc_module PRIVATE PQC_ENABLE_MLKEM_REF=${PQC_ENABLE_MLKEM_REF_VALUE})
target_compile_definitions(pqc_module PRIVATE PQC_ENABLE_MLDSA_REF=${PQC_ENABLE_MLDSA_REF_VALUE})
if(WIN32 AND PQC_ENABLE_MLKEM_REF_VALUE)
    target_link_libraries(pqc_module PRIVATE Advapi32)
endif()

add_executable(pqc_cli
    src/main.c
)

target_link_libraries(pqc_cli PRIVATE pqc_module)

include(CTest)
if(BUILD_TESTING)
    add_executable(test_kem
        tests/test_kem.c
    )
    target_link_libraries(test_kem PRIVATE pqc_module)
    add_test(NAME test_kem COMMAND test_kem)

    add_executable(test_negative
        tests/test_negative.c
    )
    target_link_libraries(test_negative PRIVATE pqc_module)
    add_test(NAME test_negative COMMAND test_negative)

    add_executable(test_ref_kem
        tests/test_ref_kem.c
    )
    target_link_libraries(test_ref_kem PRIVATE pqc_module)
    add_test(NAME test_ref_kem COMMAND test_ref_kem)

    add_executable(test_sig
        tests/test_sig.c
    )
    target_link_libraries(test_sig PRIVATE pqc_module)
    add_test(NAME test_sig COMMAND test_sig)

    add_executable(test_kat_mldsa
        tests/test_kat_mldsa.c
    )
    target_link_libraries(test_kat_mldsa PRIVATE pqc_module)
    target_compile_definitions(test_kat_mldsa PRIVATE TESTS_DATA_DIR="${CMAKE_SOURCE_DIR}/tests/data")
    add_test(NAME test_kat_mldsa COMMAND test_kat_mldsa)

    add_test(NAME cli_help COMMAND pqc_cli --help)

    add_test(
        NAME cli_missing_input_file
        COMMAND pqc_cli encaps --pub does_not_exist.key --ct out.ct --ss out.ss
    )
    set_tests_properties(cli_missing_input_file PROPERTIES WILL_FAIL TRUE)

    add_test(
        NAME cli_corrupted_key_file
        COMMAND pqc_cli decaps --sec ${CMAKE_SOURCE_DIR}/tests/data/bad.sec --ct ${CMAKE_SOURCE_DIR}/tests/data/bad.ct --ss recovered.ss
    )
    set_tests_properties(cli_corrupted_key_file PROPERTIES WILL_FAIL TRUE)
endif()
