#include "events/listener.hh"

#include "events/recvheader.hh"

namespace http
{
    void ListenerEW::operator()()
    {
        std::cout << "CB: ListenerEW\n";
        auto client_fd = sock_->accept(nullptr, nullptr);
        event_register.register_event<RecvHeaderEW>(client_fd);
    }
} // namespace http
