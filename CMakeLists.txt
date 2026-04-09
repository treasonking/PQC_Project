cmake_minimum_required(VERSION 3.16)
project(pqc_crypto_module_lab VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(pqc_module
    src/pqc_module.c
    src/pqc_kem.c
    src/pqc_utils.c
)

target_include_directories(pqc_module
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

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
