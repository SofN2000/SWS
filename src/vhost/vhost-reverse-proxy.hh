/**
 * \file vhost/vhost-reverse-proxy.hh
 * \brief VhostReverseProxy declaration.
 */

#pragma once

#include <ev.h>

#include "config/config.hh"
#include "misc/addrinfo/addrinfo-iterator.hh"
#include "misc/addrinfo/addrinfo.hh"
#include "request/request.hh"
#include "vhost/connection.hh"
#include "vhost/dispatcher.hh"
#include "vhost/load-balancer-factory.hh"
#include "vhost/load-balancer.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class VHostReverseProxy
     * \brief VHost in charge of forwarding the Request to the upstream/backend
     * service.
     */
    class VHostReverseProxy : public VHost
    {
    public:
        friend class VHostFactory;
        virtual ~VHostReverseProxy() = default;

    private:
        /**
         * \brief Constructor called by the factory.
         *
         * \param config VHostConfig virtual host configuration.
         */
        explicit VHostReverseProxy(const VHostConfig &vhost_config)
            : VHost(vhost_config)
            , load_balancer_(nullptr)
            , ip_port_opt_(std::nullopt)
        {
            if (vhost_config.proxy_pass_opt.value().upstream.empty())
            {
                auto ip = vhost_config.proxy_pass_opt.value().ip;
                auto port = vhost_config.proxy_pass_opt.value().port;
                ip_port_opt_ = std::make_optional(std::make_pair(ip, port));
            }
            else
            {
                auto upstream_name =
                    vhost_config.proxy_pass_opt.value().upstream;
                auto upstream = dispatcher.get_upstreams().at(upstream_name);
                load_balancer_ = LoadBalancerFactory::Create(upstream.hosts,
                                                             upstream.method);
            }
        }

        shared_load_balancer load_balancer_;
        std::optional<std::pair<std::string, int>> ip_port_opt_;

    public:
        /**
         * \brief Send request to the upstream.
         *
         * \param req Request.
         * \param conn std::shared_ptr<Connection>.
         */
        void respond(Request &request, shared_socket sock) final;

        /**
         * \brief Process response headers
         *
         * \param response Response.
         */
        void process_response(Response &response);

        void respond_basic(Request &request, shared_socket sock);
    };

    using shared_reverse_vhost = std::shared_ptr<VHostReverseProxy>;
} // namespace http
