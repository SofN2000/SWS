/**
 * \file vhost/dispatcher.hh
 * \brief Dispatcher declaration.
 */

#pragma once

#include <string>
#include <unordered_map>

#include "vhost/vhost-static-file.hh"

namespace http
{
    /**
     * \class Dispatcher
     * \brief Instance in charge of dispatching requests between vhosts.
     */
    class Dispatcher
    {
    public:
        Dispatcher() = default;
        Dispatcher(const Dispatcher &) = delete;
        Dispatcher &operator=(const Dispatcher &) = delete;
        Dispatcher(Dispatcher &&) = delete;
        Dispatcher &operator=(Dispatcher &&) = delete;

        void register_vhost(shared_vhost vhost);
        void dispatch(Request request, shared_socket sock) const;
        shared_vhost get_default_vhost() const;

        std::optional<shared_vhost> at_name(std::string name,
                                            ssize_t port) const;
        void set_upstreams(std::map<std::string, Upstream> v_upstreams);
        std::map<std::string, Upstream> get_upstreams();

    private:
        void register_SSL_vhost(shared_vhost vhost);
        void process_absolute_uri(Request &request) const;
        std::optional<shared_vhost> at_ip(std::string name, ssize_t port) const;
        std::unordered_map<std::string, shared_vhost> vhosts_by_name;
        std::unordered_map<std::string, shared_vhost> vhosts_by_ip;

        bool ssl_library_initialized = false;
        shared_vhost default_vhost_ = nullptr;
        std::map<std::string, Upstream> upstreams;
    };

    /**
     * \brief Service object.
     */
    extern Dispatcher dispatcher;
} // namespace http
