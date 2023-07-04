#include "vhost.hh"

#include <openssl/x509.h>

#include "dispatcher.hh"
#include "misc/openssl/ssl.hh"

#define SET_SSL_ECDH_ONOFF 1

namespace http
{
    void VHost::verify_SSL()
    {
        auto x509_cert = SSL_CTX_get0_certificate(this->ssl_ctx_.get());
        auto server_name = this->conf_.server_name.c_str();
        auto len_name = this->conf_.server_name.size();
        ssl::x509_check_host("ERR: Failed to check host from SSL certificate",
                             x509_cert, server_name, len_name, 0, nullptr);
        ssl::ctx_check_private_key("ERR: Faield to verify SSL PrivateKey",
                                   this->ssl_ctx_.get());
    }

    void VHost::setup_SSL()
    {
        this->ssl_ctx_.reset(SSL_CTX_new(TLS_method()));
        SSL_CTX_set_ecdh_auto(this->ssl_ctx_.get(), SET_SSL_ECDH_ONOFF);
        ssl::ctx_use_certificate_file(
            "ERR: Failed to load SSL certificate file", this->ssl_ctx_.get(),
            this->conf_.ssl_cert.c_str(), SSL_FILETYPE_PEM);
        ssl::ctx_use_PrivateKey_file(
            "ERR: Failed to load SSL PrivateKey file", this->ssl_ctx_.get(),
            this->conf_.ssl_key.c_str(), SSL_FILETYPE_PEM);
    }

    static int SNI_callback(SSL *ssl, int *, void *arg)
    {
        std::cout << "CB: SNI_callback\n";
        auto *vhost_arg = reinterpret_cast<VHost *>(arg);
        auto ip = vhost_arg->conf_get().ip;
        auto port = vhost_arg->conf_get().port;
        auto server_name = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
        std::cout << "CB: SNI server_name is " << server_name << '\n';
        if (server_name == NULL)
        {
            std::cout << "CB: SNI ignored\n";
            return SSL_TLSEXT_ERR_NOACK;
        }
        auto vhost = dispatcher.at_name(server_name, port);
        if (!vhost.has_value())
        {
            std::cout << "CB: SNI ignored\n";
            return SSL_TLSEXT_ERR_NOACK;
        }
        if (vhost.value()->conf_get().ssl_key.empty()
            || vhost.value()->conf_get().ip != ip)
        {
            std::cout << "CB: SNI ignored\n";
            return SSL_TLSEXT_ERR_NOACK;
        }
        auto ctx = vhost.value()->get_ssl_ctx_();
        SSL_set_SSL_CTX(ssl, ctx);
        return SSL_TLSEXT_ERR_OK;
    }

    void VHost::setup_SNI()
    {
        SSL_CTX_set_tlsext_servername_arg(this->ssl_ctx_.get(), this);
        SSL_CTX_set_tlsext_servername_callback(this->ssl_ctx_.get(),
                                               SNI_callback);
    }

    void VHost::client_sock_set(int client_sock)
    {
        this->conf_.client_sock = client_sock;
    }
} // namespace http
