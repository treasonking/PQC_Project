cmake_minimum_required(VERSION 3.16)
project(pqc_crypto_module_lab VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(PQC_MODULE_SOURCES
    src/pqc_module.c
    src/pqc_kem.c
    src/pqc_utils.c
)

set(PQCLEAN_MLKEM768_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/mlkem_pqclean/clean")
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
        ${PQCLEAN_COMMON_DIR}
)

target_compile_definitions(pqc_module PRIVATE PQC_ENABLE_MLKEM_REF=${PQC_ENABLE_MLKEM_REF_VALUE})
if(WIN32 AND PQC_ENABLE_MLKEM_REF_VALUE)
    target_link_libraries(pqc_module PRIVATE Advapi32)
endif()

add_executable(pqc_cli
    src/main.c
)

target_link_libraries(pqc_cli PRIVATE pqc_module)

add_executable(benchmark_kem
    bench/benchmark_kem.c
)
target_link_libraries(benchmark_kem PRIVATE pqc_module)

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
endif()
