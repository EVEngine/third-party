# Build-time discovery of the canonical bundled ExternalProject. Installed Poco
# packages use CMake's standard FindOpenSSL against the completed install prefix.
if(NOT TARGET OpenSSL::SSL OR NOT TARGET OpenSSL::Crypto)
    message(FATAL_ERROR "eve_add_openssl must run before configuring Poco")
endif()
set(OPENSSL_INCLUDE_DIR "${EVE_OPENSSL_PREFIX}/include")
get_target_property(OPENSSL_SSL_LIBRARY OpenSSL::SSL IMPORTED_LOCATION)
get_target_property(OPENSSL_CRYPTO_LIBRARY OpenSSL::Crypto IMPORTED_LOCATION)
set(OPENSSL_LIBRARIES "${OPENSSL_SSL_LIBRARY};${OPENSSL_CRYPTO_LIBRARY}")
set(OPENSSL_VERSION "${EVE_OPENSSL_VERSION}")
set(OpenSSL_VERSION "${OPENSSL_VERSION}")
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenSSL REQUIRED_VARS
    OPENSSL_INCLUDE_DIR OPENSSL_SSL_LIBRARY OPENSSL_CRYPTO_LIBRARY
    VERSION_VAR OPENSSL_VERSION)
set(OPENSSL_FOUND "${OpenSSL_FOUND}")
