#include <Poco/Net/Context.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/NetSSL.h>
#include <Poco/Net/SecureServerSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/StreamCopier.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace {
void require(bool condition, const char* reason) {
    if (!condition) throw std::runtime_error(reason);
}

class Handler final : public Poco::Net::HTTPRequestHandler {
    void handleRequest(Poco::Net::HTTPServerRequest&, Poco::Net::HTTPServerResponse& response) override {
        response.setContentLength(6);
        response.send() << "tls-ok";
    }
};

class Factory final : public Poco::Net::HTTPRequestHandlerFactory {
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest&) override {
        return new Handler;
    }
};

std::string get(unsigned short port, const Poco::Net::Context::Ptr& context, const std::string& host) {
    Poco::Net::HTTPSClientSession session(host, port, context);
    session.setTimeout(Poco::Timespan(5, 0));
    Poco::Net::HTTPRequest request("GET", "/", Poco::Net::HTTPMessage::HTTP_1_1);
    session.sendRequest(request);
    Poco::Net::HTTPResponse response;
    auto&                   stream = session.receiveResponse(response);
    std::ostringstream      body;
    Poco::StreamCopier::copyStream(stream, body);
    require(response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK, "HTTPS response status");
    return body.str();
}

void run() {
    // Generate an ephemeral test CA/server certificate. No network, fixture
    // expiry, openssl executable or host CA installation is required.
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> key(EVP_PKEY_Q_keygen(nullptr, nullptr, "EC", "prime256v1"),
                                                            EVP_PKEY_free);
    std::unique_ptr<X509, decltype(&X509_free)>         cert(X509_new(), X509_free);
    require(key && cert, "certificate allocation");
    require(X509_set_version(cert.get(), 2) == 1, "certificate version");
    require(ASN1_INTEGER_set(X509_get_serialNumber(cert.get()), 1) == 1, "serial");
    require(X509_gmtime_adj(X509_getm_notBefore(cert.get()), -60) != nullptr, "notBefore");
    require(X509_gmtime_adj(X509_getm_notAfter(cert.get()), 3600) != nullptr, "notAfter");
    require(X509_set_pubkey(cert.get(), key.get()) == 1, "public key");
    auto* name = X509_get_subject_name(cert.get());
    require(X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char*>("localhost"),
                                       -1, -1, 0) == 1,
            "subject");
    require(X509_set_issuer_name(cert.get(), name) == 1, "issuer");
    std::unique_ptr<X509_EXTENSION, decltype(&X509_EXTENSION_free)> san(
        X509V3_EXT_conf_nid(nullptr, nullptr, NID_subject_alt_name, const_cast<char*>("DNS:localhost")),
        X509_EXTENSION_free);
    require(san && X509_add_ext(cert.get(), san.get(), -1) == 1, "SAN");
    require(X509_sign(cert.get(), key.get(), EVP_sha256()) > 0, "certificate signature");

    Poco::Net::Context::Ptr serverContext =
        new Poco::Net::Context(Poco::Net::Context::SERVER_USE, "", "", "", Poco::Net::Context::VERIFY_NONE);
    require(SSL_CTX_use_certificate(serverContext->sslContext(), cert.get()) == 1, "server certificate");
    require(SSL_CTX_use_PrivateKey(serverContext->sslContext(), key.get()) == 1, "server key");
    Poco::Net::SecureServerSocket socket(Poco::Net::SocketAddress("127.0.0.1", 0), 8, serverContext);
    Poco::Net::HTTPServer         server(new Factory, socket, new Poco::Net::HTTPServerParams);
    server.start();
    const auto port = socket.address().port();
    for (int protocol : {TLS1_2_VERSION, TLS1_3_VERSION}) {
        Poco::Net::Context::Ptr client =
            new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_STRICT);
        require(X509_STORE_add_cert(SSL_CTX_get_cert_store(client->sslContext()), cert.get()) == 1, "client CA");
        require(SSL_CTX_set_min_proto_version(client->sslContext(), protocol) == 1, "minimum protocol");
        require(SSL_CTX_set_max_proto_version(client->sslContext(), protocol) == 1, "maximum protocol");
        require(get(port, client, "localhost") == "tls-ok", "HTTPS payload");
        std::cout << "Verified HTTPS with TLS " << (protocol == TLS1_2_VERSION ? "1.2" : "1.3") << '\n';
    }
    Poco::Net::Context::Ptr untrusted =
        new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_STRICT);
    bool rejected = false;
    try {
        get(port, untrusted, "localhost");
    } catch (const Poco::Exception&) {
        rejected = true;
    }
    require(rejected, "untrusted certificate was accepted");
    Poco::Net::Context::Ptr wrongHost =
        new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_STRICT);
    require(X509_STORE_add_cert(SSL_CTX_get_cert_store(wrongHost->sslContext()), cert.get()) == 1, "hostname test CA");
    rejected = false;
    try {
        get(port, wrongHost, "127.0.0.1");
    } catch (const Poco::Exception&) {
        rejected = true;
    }
    require(rejected, "hostname mismatch was accepted");
    server.stopAll();
    std::cout << "Rejected untrusted certificate and hostname mismatch\n";
}
}  // namespace

int main() {
    try {
        Poco::Net::initializeSSL();
        run();
        Poco::Net::uninitializeSSL();
        return 0;
    } catch (const Poco::Exception& error) {
        std::cerr << error.displayText() << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
