#include "recv-response-header.hh"

#include "events/recv-response-body.hh"
#include "events/register.hh"
#include "events/send-response.hh"
#include "request/error.hh"
#include "vhost/vhost-reverse-proxy.hh"

namespace http
{
    bool RecvResponseHeaderEW::is_complete()
    {
        std::cout << "BEFOREE: " << this->header_ << '\n';
        auto double_crlf = std::string{ "\r\n\r\n" };
        std::cout << "$ is_complete in recv-response-header" << '\n';
        auto end_of_header = this->header_.find(double_crlf);
        if (end_of_header == std::string::npos)
            return false;
        auto body_start =
            this->header_.substr(end_of_header + double_crlf.size());
        this->response_.body.append(body_start);
        this->header_.erase(end_of_header + double_crlf.size());
        std::cout << "AFTER: " << this->header_ << '\n';
        std::cout << "BODY: " << this->response_.body << '\n';
        return true;
    }

    static void remove_trailing_crlf(std::string &line)
    {
        line = line.substr(0, line.size() - 2);
    }

    void RecvResponseHeaderEW::check_and_stop_timer()
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

    void RecvResponseHeaderEW::parse_header()
    {
        std::cout << "$ parse_header\n";
        event_register.unregister_ew(this);
        remove_trailing_crlf(this->header_);
        try
        {
            auto body_tmp = this->response_.body;
            this->response_ = process_response(Response(this->header_));
            this->response_.body = body_tmp;
            if (this->method_ == Request::GET || this->method_ == Request::POST)
            {
                auto content_iter =
                    this->response_.header_fields.find("Content-Length");
                auto body_length = this->response_.body.length();
                if (content_iter != this->response_.header_fields.end())
                {
                    auto content_length =
                        static_cast<size_t>(std::stoi(content_iter->second));
                    std::cout << "got: " << body_length
                              << " expected: " << content_length << '\n';
                    if (body_length == content_length)
                    {
                        check_and_stop_timer();
                        auto vhost_rev_proxy =
                            dynamic_cast<VHostReverseProxy *>(
                                this->vhost_.get());
                        vhost_rev_proxy->process_response(this->response_);
                        this->response_.str = this->response_.to_string();
                        event_register.register_event<SendResponseEW>(
                            this->client_sock_, this->response_);
                    }
                    else
                    {
                        auto vhost_rev_proxy =
                            dynamic_cast<VHostReverseProxy *>(
                                this->vhost_.get());
                        vhost_rev_proxy->process_response(this->response_);
                        this->response_.str = this->response_.to_string();
                        event_register.register_event<RecvResponseBodyEW>(
                            this->client_sock_, this->backend_sock_,
                            this->vhost_, this->response_);
                    }
                }
                else
                {
                    throw std::runtime_error(
                        "WHY IS THERE NO CONTENT-LENGTH?\n");
                }
            }
            else
            {
                auto vhost_rev_proxy =
                    dynamic_cast<VHostReverseProxy *>(this->vhost_.get());
                vhost_rev_proxy->process_response(this->response_);
                this->response_.str = this->response_.to_string();
                check_and_stop_timer();
                event_register.register_event<SendResponseEW>(
                    this->client_sock_, this->response_);
            }
        }
        catch (std::exception &e)
        {
            std::cout << "I AM THE FAILURE : " << e.what() << '\n';
            event_register.register_event<SendResponseEW>(this->client_sock_,
                                                          error::bad_gateway());
        }
    }

    void RecvResponseHeaderEW::operator()()
    {
        std::cout << "CB: RecvResponseHeaderEW\n";
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
                this->header_.append(buffer, read);
                std::cout << this->header_;
                if (this->is_complete())
                    this->parse_header();
            }
        }
        catch (std::exception &e)
        {
            event_register.unregister_ew(this);
        }
    }

    Response RecvResponseHeaderEW::process_response(Response response)
    {
        auto set_header = vhost_->conf_get().proxy_pass_opt->set_header;
        auto rm_header = vhost_->conf_get().proxy_pass_opt->remove_header;
        for (const auto &header : rm_header)
            response.header_fields.erase(header);
        for (const auto &header : set_header)
        {
            std::cout << "!!!!!!!!!!!!!!:  " << header.first
                      << "???????????????: " << header.second << '\n';
            if (response.header_fields.find(header.first)
                != response.header_fields.end())
                response.header_fields.erase(header.first);
            response.header_fields.insert(header);
            std::cout << "§§§§§§§§§§§§§§§§§§§§§§: "
                      << (response.header_fields.find(header.first))->first
                      << "??????????????????: "
                      << (response.header_fields.find(header.first))->second
                      << '\n';
        }
        return response;
    }
} // namespace http
