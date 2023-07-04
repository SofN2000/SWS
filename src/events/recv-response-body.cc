#include "recv-response-body.hh"

#include "events/register.hh"
#include "events/send-response.hh"
#include "request/error.hh"
#include "vhost/vhost-reverse-proxy.hh"

namespace http
{
    bool RecvResponseBodyEW::is_body_complete()
    {
        std::cout << "$is_body_complete\n";
        auto content_iter =
            this->response_.header_fields.find("Content-Length");
        auto body_length = this->response_.body.length();
        if (content_iter != this->response_.header_fields.end())
        {
            auto content_length =
                static_cast<size_t>(std::stoi(content_iter->second));
            if (body_length == content_length)
                return true;
            else if (body_length < content_length)
                return false;
        }
        throw std::runtime_error("bad_gateway: response invalid");
    }

    void RecvResponseBodyEW::check_and_stop_timer()
    {
        if (this->vhost_->conf_get().proxy_pass_opt.has_value())
        {
            if (this->vhost_->conf_get()
                    .proxy_pass_opt.value()
                    .transaction_timeout.has_value())
            {
                event_register.stop_timer(&this->vhost_->transaction_timer);
            }
        }
    }

    void RecvResponseBodyEW::operator()()
    {
        std::cout << "CB: RecvResponseBodyEW\n";
        char buffer[DEFAULT_BUFFER_SIZE];
        try
        {
            auto read = backend_sock_->recv(&buffer, DEFAULT_BUFFER_SIZE);
            if (read == 0)
            {
                check_and_stop_timer();
                event_register.unregister_ew(this);
            }
            else
            {
                this->response_.body.append(buffer, read);
                std::cout << this->response_.body;
                if (this->is_body_complete())
                {
                    check_and_stop_timer();
                    auto vhost_rev_proxy =
                        dynamic_cast<VHostReverseProxy *>(this->vhost_.get());
                    event_register.unregister_ew(this);
                    vhost_rev_proxy->process_response(this->response_);
                    this->response_.str = this->response_.to_string();
                    event_register.register_event<SendResponseEW>(
                        this->client_sock_, this->response_);
                }
            }
        }
        catch (std::exception &e)
        {
            event_register.register_event<SendResponseEW>(this->client_sock_,
                                                          error::bad_gateway());
            event_register.unregister_ew(this);
        }
    }
} // namespace http
