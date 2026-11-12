#include <openssl/ssl.h>

int main(void) {
    SSL_CTX* context = SSL_CTX_new(TLS_method());
    if (!context) {
        return 1;
    }
    SSL_CTX_free(context);
    return 0;
}
