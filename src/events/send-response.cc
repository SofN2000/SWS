#include "events/send-response.hh"

#include "events/recvheader.hh"

namespace http
{
    void SendResponseEW::operator()()
    {
        std::cout << "CB: SendResponseEW\n";
        std::cout << this->response_.str;
        auto sent = sock_->send(this->response_.str.c_str(),
                                this->response_.str.size());
        this->response_.str.erase(0, sent);
        if (this->response_.str.size() == 0)
            event_register.unregister_ew(this);
        if (this->response_.connect == connection::KEEPALIVE)
        {
            std::cout << "connection is keep-alive!\n";
            event_register.register_event<RecvHeaderEW>(this->sock_);
        }
    }
} // namespace http
