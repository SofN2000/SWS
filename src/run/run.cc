#include "run/run.hh"

#include <charconv>
#include <iostream>

#include "config/config.hh"
#include "events/event-loop.hh"
#include "events/events.hh"
#include "events/listener.hh"
#include "events/register.hh"
#include "misc/addrinfo/addrinfo.hh"
#include "misc/fd.hh"
#include "misc/json.hh"
#include "misc/socket.hh"
#include "socket/default-socket.hh"
#include "socket/ssl-socket.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost-factory.hh"

namespace http
{
    static shared_socket create_and_bind(const addrinfo &ai)
    {
        auto sock_ptr = std::make_shared<DefaultSocket>(
            ai.ai_family, ai.ai_socktype, ai.ai_protocol);
        sock_ptr->setsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 1);
        sock_ptr->bind(ai.ai_addr, ai.ai_addrlen);
        return sock_ptr;
    }

    static shared_socket create_and_bind_ssl(const addrinfo &ai,
                                             const shared_vhost &vhost)
    {
        auto sock_ptr =
            std::make_shared<SSLSocket>(ai.ai_family, ai.ai_socktype,
                                        ai.ai_protocol, vhost->get_ssl_ctx_());
        sock_ptr->setsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 1);
        sock_ptr->bind(ai.ai_addr, ai.ai_addrlen);
        return sock_ptr;
    }

    static shared_socket try_create_and_bind(const misc::AddrInfo &addrinfo,
                                             const bool &ssl,
                                             const shared_vhost &vhost)
    {
        for (const auto &ai : addrinfo)
        {
            try
            {
                if (!ssl)
                    return create_and_bind(ai);
                else
                    return create_and_bind_ssl(ai, vhost);
            }
            catch (...)
            {
                continue;
            }
        }
        throw std::runtime_error("Failed to create and bind socket");
    }

    static shared_socket create_server_socket(const shared_vhost &vhost)
    {
        misc::AddrInfoHint hints;
        hints.family(AF_UNSPEC);
        hints.socktype(SOCK_STREAM);
        hints.flags(AI_PASSIVE);
        auto config = vhost->conf_get();
        auto port = std::to_string(config.port);
        auto ai = misc::getaddrinfo(config.ip.c_str(), port.c_str(), hints);
        shared_socket socket;
        if (!config.ssl_key.empty())
        {
            socket = try_create_and_bind(ai, true, vhost);
            socket->listen(20);
        }
        else
        {
            socket = try_create_and_bind(ai, false, vhost);
            socket->listen(20);
        }
        return socket;
    }

    static void register_vhosts(const ServerConfig &server_config)
    {
        dispatcher.set_upstreams(server_config.upstreams);
        for (const auto &vhost_config : server_config.vhosts)
        {
            auto vhost = VHostFactory::Create(vhost_config);
            dispatcher.register_vhost(vhost);
            auto vhost_socket = create_server_socket(vhost);
            event_register.register_event<ListenerEW>(vhost_socket);
        }
    }

    void run(const ServerConfig &server_config, char **argv)
    {
        std::cout << "$ run\n";
        register_vhosts(server_config);
        event_register.run(argv);
    }

} // namespace http
