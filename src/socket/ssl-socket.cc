#define DEFAULT_BUFFER_SIZE 256

#include "ssl-socket.hh"

#include <memory>
#include <openssl/ssl.h>
#include <unistd.h>

#include "misc/openssl/ssl.hh"

namespace http
{
    SSLSocket::SSLSocket(int domain, int type, int protocol, SSL_CTX *ssl_ctx)
    {
        this->fd_ = std::make_shared<misc::FileDescriptor>(
            sys::socket(domain, type, protocol));
        sys::fcntl_set(fd_->fd_, O_NONBLOCK);
        auto server_ssl = SSL_new(ssl_ctx);
        this->ssl_.reset(server_ssl);
    }

    SSLSocket::SSLSocket(const misc::shared_fd &fd, SSL_CTX *ssl_ctx)
    {
        this->fd_ = fd;
        auto client_ssl = SSL_new(ssl_ctx);
        this->ssl_.reset(client_ssl);
        auto set_fd = SSL_set_fd(this->ssl_.get(), this->fd_->fd_);
        if (set_fd <= 0)
        {
            throw std::runtime_error("Cannot set client_fd for SSL object!");
        }
        std::cout << "SSL: about to accept\n";
        ssl::accept(this->ssl_.get());
    }

    void SSLSocket::bind(const sockaddr *addr, socklen_t addrlen)
    {
        sys::bind(this->fd_->fd_, addr, addrlen);
    }

    shared_socket SSLSocket::accept(sockaddr *addr, socklen_t *addrlen)
    {
        auto client_fd = sys::accept(this->fd_->fd_, addr, addrlen);
        auto fd_ptr = std::make_shared<misc::FileDescriptor>(client_fd.fd_);
        client_fd.fd_ = -1;
        auto ssl_ctx = SSL_get_SSL_CTX(this->ssl_.get()); // server's SSL object
        return std::make_shared<SSLSocket>(fd_ptr, ssl_ctx);
    }

    void SSLSocket::listen(int backlog)
    {
        sys::listen(this->fd_->fd_, backlog);
    }

    ssize_t SSLSocket::sendfile(misc::shared_fd &src, off_t &offset,
                                size_t count)
    {
        auto fd_dup = dup(src->fd_);
        auto file_in = fdopen(fd_dup, "r");
        if (offset != 0)
            fseek(file_in, offset, SEEK_SET);
        char buf[DEFAULT_BUFFER_SIZE];
        auto read = fread(&buf, sizeof(char), DEFAULT_BUFFER_SIZE, file_in);
        if (read < DEFAULT_BUFFER_SIZE && ferror(file_in) && !feof(file_in))
            return -1;
        if (read > count)
            fseek(file_in, count - read, SEEK_CUR);
        auto s = this->send(&buf, read);
        fclose(file_in);
        if (s <= 0)
            return -1;
        offset += read;
        return read;
    }

    void SSLSocket::connect(const sockaddr *addr, socklen_t addrlen)
    {
        sys::connect(this->fd_->fd_, addr, addrlen);
        ssl::connect(this->ssl_.get());
    }

    ssize_t SSLSocket::recv(void *buf, size_t len)
    {
        return ssl::read(this->ssl_.get(), buf, len);
    }

    ssize_t SSLSocket::send(const void *buf, size_t len)
    {
        return ssl::write(this->ssl_.get(), buf, len);
    }

    void SSLSocket::setsockopt(int level, int optname, int optval)
    {
        socklen_t optlen = sizeof(int);
        sys::setsockopt(this->fd_->fd_, level, optname, &optval, optlen);
    }

    void SSLSocket::getsockopt(int level, int optname, int &optval)
    {
        socklen_t optlen;
        sys::getsockopt(this->fd_->fd_, level, optname, &optval, &optlen);
    }
} // namespace http
