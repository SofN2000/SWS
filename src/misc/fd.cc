#include "misc/fd.hh"

#include <iostream>

#include "misc/unistd.hh"

namespace misc
{
    FileDescriptor::~FileDescriptor()
    {
        if (this->fd_ > -1)
        {
            std::cout << "<<< FileDescriptor " << this->fd_ << "\n";
            sys::close(this->fd_);
        }
    }

    FileDescriptor &FileDescriptor::operator=(FileDescriptor &&fileDescriptor)
    {
        std::cout << "=== FileDescriptor " << this->fd_ << "\n";
        if (this->fd_ > -1)
            sys::close(this->fd_);
        this->fd_ = std::exchange(fileDescriptor.fd_, -1);
        return *this;
    }

    FileDescriptor::operator int() const &
    {
        return this->fd_;
    }

    FileDescriptor::operator bool() const &
    {
        return this->fd_ > -1;
    }

} // namespace misc
