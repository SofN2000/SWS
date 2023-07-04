#include "events/recvbody.hh"

namespace http
{
    void RecvBodyEW::operator()()
    {
        std::cout << "CB: RecvBodyEW\n";
        char buffer[DEFAULT_BUFFER_SIZE];
        auto read = sock_->recv(&buffer, DEFAULT_BUFFER_SIZE);
        if (read == 0)
        {
            event_register.unregister_ew(this);
        }
        else
        {
            this->request_.body.append(buffer, read);
            event_register.unregister_ew(this);
            dispatcher.dispatch(this->request_, this->sock_);
        }
    }
} // namespace http
