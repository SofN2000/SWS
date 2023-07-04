#include "socket/default-socket.hh"

#include "misc/socket.hh"

namespace http
{
    DefaultSocket::DefaultSocket(int domain, int type, int protocol)
        : Socket{ std::make_shared<misc::FileDescriptor>(
            sys::socket(domain, type, protocol)) }
    {
        sys::fcntl_set(fd_->fd_, O_NONBLOCK);
    }

    ssize_t DefaultSocket::recv(void *buf, size_t len)
    {
        return sys::recv(fd_->fd_, buf, len, 0);
    }

    ssize_t DefaultSocket::send(const void *buf, size_t len)
    {
        return sys::send(fd_->fd_, buf, len, MSG_NOSIGNAL);
    }

    ssize_t DefaultSocket::sendfile(misc::shared_fd &in_fd, off_t &offset,
                                    size_t count)
    {
        return sys::sendfile(fd_->fd_, in_fd->fd_, &offset, count);
    }

    void DefaultSocket::bind(const sockaddr *addr, socklen_t addrlen)
    {
        sys::bind(fd_->fd_, addr, addrlen);
    }

    void DefaultSocket::listen(int backlog)
    {
        sys::listen(fd_->fd_, backlog);
    }

    void DefaultSocket::setsockopt(int level, int optname, int optval)
    {
        socklen_t optlen = sizeof(int);
        sys::setsockopt(fd_->fd_, level, optname, &optval, optlen);
    }

    void DefaultSocket::getsockopt(int level, int optname, int &optval)
    {
        socklen_t optlen;
        sys::getsockopt(fd_->fd_, level, optname, &optval, &optlen);
    }
    shared_socket DefaultSocket::accept(sockaddr *addr, socklen_t *addrlen)
    {
        auto fd = sys::accept(fd_->fd_, addr, addrlen);
        auto fd_ptr = std::make_shared<misc::FileDescriptor>(fd.fd_);
        fd.fd_ = -1;
        return std::make_shared<DefaultSocket>(fd_ptr);
    }

    void DefaultSocket::connect(const sockaddr *addr, socklen_t addrlen)
    {
        sys::connect(fd_->fd_, addr, addrlen);
    }

} // namespace http
