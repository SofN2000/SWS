#include "dispatcher.hh"

#include <algorithm>
#include <openssl/ssl.h>
#include <regex>
#include <sstream>

#include "events/register.hh"
#include "events/send-response.hh"
#include "misc/inet.hh"
#include "request/error.hh"
#include "vhost-reverse-proxy.hh"

namespace http
{
    Dispatcher dispatcher;

    void Dispatcher::register_vhost(shared_vhost vhost)
    {
        auto host = vhost->conf_get().server_name;
        auto ip = vhost->conf_get().ip;
        vhosts_by_name.insert(std::make_pair(host, vhost));
        vhosts_by_ip.insert(std::make_pair(ip, vhost));
        if (vhost->conf_get().is_default)
            this->default_vhost_ = vhost;
        if (!vhost->conf_get().ssl_key.empty())
            register_SSL_vhost(vhost);
    }

    static void initialize_SSL()
    {
        SSL_load_error_strings();
        SSL_library_init();
        OpenSSL_add_all_algorithms();
    }

    void Dispatcher::register_SSL_vhost(shared_vhost vhost)
    {
        if (!this->ssl_library_initialized)
        {
            initialize_SSL();
            this->ssl_library_initialized = true;
        }
        vhost->setup_SSL();
        vhost->verify_SSL();
        vhost->setup_SNI();
    }

    static ssize_t extract_port(std::string &host)
    {
        if (*host.begin() == '[')
        {
            auto closing = host.find(']');
            if (closing == std::string::npos)
            {
                throw std::runtime_error("no closing bracket");
            }
            auto ipv6 = host.substr(0, closing + 1);
            if (!is_ipv6_addr(ipv6))
                throw std::runtime_error("invalid ipv6");
            if (closing == host.size() - 1)
                return -1;
            auto port = host.substr(closing + 1);
            host.erase(closing + 1);
            if (*port.begin() == ':')
            {
                return std::stoi(port.substr(1));
            }
            throw std::runtime_error("invalid string");
        }
        else
        {
            auto colon = host.find(':');
            if (colon == std::string::npos)
                return -1;
            auto iss = std::istringstream(host);
            auto name_or_ip = std::string{};
            std::getline(iss, name_or_ip, ':');
            auto port_str = std::string{};
            std::getline(iss, port_str);
            auto port = std::stoi(port_str);
            host.erase(colon);
            return port;
        }
    }

    std::optional<shared_vhost> Dispatcher::at_name(std::string name,
                                                    ssize_t port) const
    {
        try
        {
            auto vhost = this->vhosts_by_name.at(name);
            if (port != -1 && vhost->conf_get().port != port)
                throw std::out_of_range("port doesn't match");
            return vhost;
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    std::optional<shared_vhost> Dispatcher::at_ip(std::string ip,
                                                  ssize_t port) const
    {
        try
        {
            if (*ip.begin() == '[')
            {
                ip = ip.substr(1, ip.size() - 2);
            }
            std::cout << "IP after removed square bracket: " << ip << std::endl;
            auto vhost = this->vhosts_by_ip.at(ip);
            if (port != -1 && vhost->conf_get().port != port)
                throw std::out_of_range("port doesn't match");
            return vhost;
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    static bool is_absolute_uri(const std::string &target)
    {
        return target.find("://") != std::string::npos;
    }

    static bool is_valid_absolute_uri(std::string &uri)
    {
        const auto http_regex = std::regex{ "http://.*" };
        const auto https_regex = std::regex{ "https://.*" };
        if (std::regex_match(uri, http_regex))
        {
            uri.erase(0, 7);
            return true;
        }
        if (std::regex_match(uri, https_regex))
        {
            uri.erase(0, 8);
            return true;
        }
        return false;
    }

    void Dispatcher::process_absolute_uri(Request &request) const
    {
        auto iss = std::istringstream(request.target);
        auto host = std::string{};
        std::getline(iss, host, '/');
        std::getline(iss, request.target);
        request.target.insert(0, 1, '/');
        request.header_fields.insert_or_assign("Host", host);
    }

    static void register_bad_request_reponse(const Request &request,
                                             shared_socket sock)
    {
        event_register.register_event<SendResponseEW>(
            sock, error::bad_request(request));
    }

    void Dispatcher::dispatch(Request request, shared_socket sock) const
    {
        std::cout << "$ dispatch\n";
        if (is_absolute_uri(request.target))
        {
            if (is_valid_absolute_uri(request.target))
                process_absolute_uri(request);
            else
                return register_bad_request_reponse(request, sock);
        }
        auto host_opt = request.at("Host");
        if (!host_opt.has_value())
        {
            register_bad_request_reponse(request, sock);
            return;
        }
        auto host = host_opt.value();
        try
        {
            auto port = extract_port(host);
            auto vhost = this->at_ip(host, port);
            if (!vhost.has_value())
                vhost = this->at_name(host, port);
            if (vhost.has_value())
            {
                auto cl = vhost.value()->conf_get().client_sock;
                if (request.connect == connection::KEEPALIVE
                    && (cl == -1 || cl == sock->fd_get()->fd_))
                {
                    vhost.value()->client_sock_set(sock->fd_get()->fd_);
                    vhost.value()->respond(request, sock);
                    return;
                }
                else if (request.connect == connection::CLOSE)
                {
                    vhost.value()->respond(request, sock);
                }
                else
                {
                    register_bad_request_reponse(request, sock);
                }
            }
            else
                register_bad_request_reponse(request, sock);
        }
        catch (std::exception &e)
        {
            std::cout << "WHAT THE HELL " << e.what() << '\n';
            auto vhost = default_vhost_;
            if (vhost != nullptr)
                vhost->respond(request, sock);
            else
                register_bad_request_reponse(request, sock);
        }
    }

    shared_vhost Dispatcher::get_default_vhost() const
    {
        return this->default_vhost_;
    }

    void Dispatcher::set_upstreams(std::map<std::string, Upstream> v_upstreams)
    {
        this->upstreams = v_upstreams;
    }
    std::map<std::string, Upstream> Dispatcher::get_upstreams()
    {
        std::cout << "CREATION of SendRequestEW" << std::endl;
        return this->upstreams;
    }

} // namespace http
