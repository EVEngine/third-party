cmake_minimum_required(VERSION 3.21)
project(OpenSSLAndroidContract C)

if(NOT ANDROID)
    message(FATAL_ERROR "Configure this contract with the Android NDK toolchain")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OpenSSL.cmake")
eve_add_openssl()

# Link against the Android libraries; no host SSL discovery or emulated run.
add_executable(openssl_android_link main.c)
target_link_libraries(openssl_android_link PRIVATE OpenSSL::SSL)
