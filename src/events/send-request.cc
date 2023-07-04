#include "send-request.hh"

#include <sstream>
#include <string>

#include "config/config.hh"
#include "events/listener.hh"
#include "events/recv-response-header.hh"
#include "events/register.hh"
#include "misc/addrinfo/addrinfo.hh"
#include "misc/inet.hh"

namespace http
{
    static shared_socket create_and_connect(const misc::AddrInfo &addrinfo)
    {
        shared_socket global;
        for (const auto &ai : addrinfo)
        {
            try
            {
                auto socket = std::make_shared<DefaultSocket>(
                    ai.ai_family, ai.ai_socktype, ai.ai_protocol);
                global = socket;
                socket->connect(ai.ai_addr, ai.ai_addrlen);
                return socket;
            }
            catch (std::exception &e)
            {
                if (std::string(e.what()) == "Operation now in progress")
                    return global;
                continue;
            }
        }
        throw std::runtime_error("Failed to create socket connect to backend");
    }

    shared_socket SendRequestEW::create_server_socket(const std::string ip,
                                                      const int port)
    {
        misc::AddrInfoHint hints;
        hints.family(AF_UNSPEC);
        hints.socktype(SOCK_STREAM);
        auto str_port = std::to_string(port);
        auto ai = misc::getaddrinfo(ip.c_str(), str_port.c_str(), hints);
        auto socket = create_and_connect(ai);
        return socket;
    }

    void SendRequestEW::operator()()
    {
        std::cout << "CB: SendRequestEW" << std::endl;
        std::cout << this->str_;
        auto sent = backend_sock_->send(this->str_.c_str(), this->str_.size());
        this->str_.erase(0, sent);
        if (this->str_.size() == 0)
        {
            event_register.register_event<RecvResponseHeaderEW>(
                this->client_sock_, this->backend_sock_, this->vhost_,
                this->request_.method);
            event_register.unregister_ew(this);
        }
    }

    void SendRequestEW::str_from_request()
    {
        switch (this->request_.method)
        {
        case Request::method::GET:
            this->str_ += "GET ";
            break;
        case Request::method::HEAD:
            this->str_ += "HEAD ";
            break;
        case Request::method::POST:
            this->str_ += "POST ";
            break;
        case Request::method::NOT_ALLOW:
        default:
            break;
        }

        this->str_ +=
            this->request_.target + ' ' + this->request_.version + "\r\n";
        for (const auto &header : this->request_.header_fields)
        {
            this->str_ += header.first + ": " + header.second + "\r\n";
        }
        this->str_ += "\r\n" + this->request_.body;
    }

    static bool
    do_forward(const std::unordered_map<std::string, std::string> header_fields)
    {
        return header_fields.find("X-Forwarded-For") != header_fields.end()
            || header_fields.find("X-Forwarded-Host") != header_fields.end()
            || header_fields.find("X-Forwarded-Proto") != header_fields.end();
    }

    Request SendRequestEW::process_request(Request request,
                                           std::string backend_ip)
    {
        std::string for_value;
        bool is_forward = do_forward(request.header_fields);
        bool host_header = true;
        if (is_forward)
        {
            if (request.header_fields.find("X-Forwarded-For")
                != request.header_fields.end())
            {
                std::stringstream iss(
                    request.header_fields.find("X-Forwarded-For")->second);
                std::string token;
                while (std::getline(iss, token, ','))
                {
                    token.erase(remove_if(token.begin(), token.end(), isspace),
                                token.end());
                    if (!for_value.empty())
                        for_value += ", ";
                    for_value += "for=";
                    if (is_ipv6_addr_no_brackets(token))
                        for_value += "\"[" + token + "]\"";
                    else if (is_ipv6_addr(token))
                        for_value += '"' + token + '"';
                    else
                        for_value += token;
                }
                request.header_fields.erase("X-Forwarded-For");
            }
            if (request.header_fields.find("X-Forwarded-Host")
                != request.header_fields.end())
            {
                for_value +=
                    request.header_fields.find("X-Forwarded-Host")->second;
                request.header_fields.erase("X-Forwarded-Host");
            }
            else
                host_header = false;
        }
        auto set_header = vhost_->conf_get().proxy_pass_opt->proxy_set_header;
        auto rm_header = vhost_->conf_get().proxy_pass_opt->proxy_remove_header;
        for (const auto &header : rm_header)
            request.header_fields.erase(header);
        if (set_header.find("Host") == set_header.end())
        {
            if (request.header_fields.find("Host")
                != request.header_fields.end())
            {
                if (!host_header)
                {
                    for_value +=
                        ";host=" + request.header_fields.find("Host")->second;
                    host_header = true;
                }
                request.header_fields.erase("Host");
            }
            request.header_fields.insert(std::make_pair("Host", backend_ip));
            if (!host_header)
                for_value +=
                    ";host=" + request.header_fields.find("Host")->second;
        }
        for (const auto &header : set_header)
        {
            if (request.header_fields.find(header.first)
                != request.header_fields.end())
                request.header_fields.erase(header.first);
            request.header_fields.insert(header);
        }
        if (is_forward)
        {
            if (request.header_fields.find("X-Forwarded-Proto")
                != request.header_fields.end())
            {
                if (this->vhost_->conf_get().ssl_key.empty())
                    for_value += ";proto=http";
                for_value += ";proto=https";
                request.header_fields.erase("X-Forwarded-Proto");
            }
            request.header_fields.insert(
                std::make_pair("Forwarded", for_value));
        }
        return request;
    }
} // namespace http
