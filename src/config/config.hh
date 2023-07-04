/**
 * \file config/config.hh
 * \brief Declaration of ServerConfig and VHostConfig.
 */

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "misc/json.hh"

namespace http
{
    /**
     * \struct HostConfig
     * \brief Value object storing upstream host config
     */
    enum class LoadBalancingMethod
    {
        ROUND_ROBIN,
        FAILOVER,
        FAIL_ROBIN
    };

    struct HostConfig
    {
        HostConfig(const json &host, const LoadBalancingMethod method)
            : ip(host.at("ip"))
            , port(host.at("port"))
            , weight(host.value("weight", 1))
        {
            if (method == LoadBalancingMethod::FAILOVER
                || method == LoadBalancingMethod::FAIL_ROBIN)
                health = host.at("health");
        }

        std::string ip;
        int port;
        uint weight;
        std::string health;
    };

    LoadBalancingMethod lb_method_of_string(const std::string &method);

    /**
     * \struct Upstream
     * \brief Value object storing upstream data
     */
    struct Upstream
    {
        Upstream() = default;

        Upstream(const json &upstream)
            : method(lb_method_of_string(upstream.at("method")))
            , hosts{}
        {
            for (const auto &host : upstream.at("hosts"))
                hosts.push_back(HostConfig(host, method));
            if (hosts.size() == 0)
                throw std::runtime_error("Bad upstream!");
        }
        LoadBalancingMethod method;
        std::vector<HostConfig> hosts;
    };

    /**
     * \struct ProxyPass
     * \brief Value object storing Proxy-pass options.
     */
    struct ProxyPass
    {
        ProxyPass() = default;

        ProxyPass(const json &proxy_pass)
            : ip(proxy_pass.value("ip", ""))
            , port(proxy_pass.value("port", -1))
            , upstream(proxy_pass.value("upstream", ""))
            , proxy_set_header{}
            , proxy_remove_header{}
            , set_header{}
            , remove_header{}
        {
            if (proxy_pass.contains("upstream"))
            {
                if (proxy_pass.contains("ip") || proxy_pass.contains("port"))
                    throw std::runtime_error(
                        "Upstream and IP/Port are mutually exclusive");
            }
            if (proxy_pass.contains("proxy_set_header"))
            {
                auto json = proxy_pass.at("proxy_set_header").items();
                for (const auto &key_val : json)
                    proxy_set_header.insert(
                        std::make_pair(key_val.key(), key_val.value()));
            }
            if (proxy_pass.contains("set_header"))
            {
                auto json = proxy_pass.at("set_header").items();
                for (const auto &key_val : json)
                    set_header.insert(
                        std::make_pair(key_val.key(), key_val.value()));
            }
            if (proxy_pass.contains("proxy_remove_header"))
            {
                auto json = proxy_pass.at("proxy_remove_header");
                for (const auto &header : json)
                    proxy_remove_header.push_back(header);
            }
            if (proxy_pass.contains("remove_header"))
            {
                auto json = proxy_pass.at("remove_header");
                for (const auto &header : json)
                    remove_header.push_back(header);
            }
            if (proxy_pass.contains("timeout"))
            {
                this->transaction_timeout = proxy_pass.at("timeout");
                if (this->transaction_timeout.value() < 0)
                    throw std::invalid_argument(
                        "proxy timeout must be positive float");
            }
        }

        /// The IP of the proxied server
        std::string ip;

        /// The port of the proxied server
        int port;

        /// Upstream object's name
        std::string upstream;

        /// Map of headers to add or set to the request
        std::unordered_map<std::string, std::string> proxy_set_header;

        /// List of headers to remove from the request
        std::vector<std::string> proxy_remove_header;

        /// Map of headers to add to the response
        std::unordered_map<std::string, std::string> set_header;

        /// List of headers to remove from the response
        std::vector<std::string> remove_header;

        /// Timeout of proxy's transaction
        std::optional<float> transaction_timeout = std::nullopt;
    };

    /**
     * \struct VHostConfig
     * \brief Value object storing a virtual host configuration.
     *
     * Since each virtual host of the server has its own configuration, a
     * dedicated structure is required to store the information related to
     * each one of them.
     */
    struct VHostConfig
    {
        VHostConfig(const json &vhost)
            : ip(vhost.at("ip"))
            , port(vhost.at("port"))
            , server_name(vhost.at("server_name"))
            , root(vhost.value("root", ""))
            , default_file(vhost.value("default_file", ""))
            , ssl_key(vhost.value("ssl_key", ""))
            , ssl_cert(vhost.value("ssl_cert", ""))
            , proxy_pass_opt(std::nullopt)
            , auth_basic(vhost.value("auth_basic", ""))
            , auth_basic_users(
                  vhost.value("auth_basic_users", std::vector<std::string>()))
            , is_default(vhost.value("default_vhost", false))
            , auto_index(vhost.value("auto_index", false))
        {
            if (vhost.contains("proxy_pass"))
            {
                if (vhost.contains("root"))
                    throw std::runtime_error(
                        "proxy_pass and root are mutually exclusive");
                if (vhost.contains("default_file"))
                    throw std::runtime_error(
                        "proxy_pass and default_file are mutually exclusive");
                if (vhost.contains("auto_index") && this->auto_index)
                    throw std::runtime_error(
                        "proxy_pass and auto_index are mutually exclusive");
                proxy_pass_opt = ProxyPass(vhost.at("proxy_pass"));
            }
            if (this->auto_index == false && this->default_file.empty())
            {
                this->default_file = "index.html";
            }
        }

        VHostConfig(const VHostConfig &) = default;
        VHostConfig &operator=(const VHostConfig &) = default;
        VHostConfig(VHostConfig &&) = default;
        VHostConfig &operator=(VHostConfig &&) = default;

        ~VHostConfig() = default;

        bool ssl_is_valid() const;
        bool auth_basic_is_valid() const;

        std::string ip;
        int port;

        std::string server_name;
        std::string root;
        std::string default_file; // Optional

        std::string ssl_key;
        std::string ssl_cert;

        std::optional<ProxyPass> proxy_pass_opt;

        std::string auth_basic;
        std::vector<std::string> auth_basic_users;

        bool is_default;
        bool auto_index = false;
        int client_sock = -1;
    };

    /**
     * \struct ServerConfig
     * \brief Value object storing the server configuration.
     *
     * To avoid opening the configuration file each time we need to access the
     * server configuration, a dedicated structure is required to store it.
     */
    struct ServerConfig
    {
        ServerConfig() = default;
        ServerConfig(const std::string &path);
        ServerConfig(const ServerConfig &) = default;
        ServerConfig &operator=(const ServerConfig &) = default;
        ServerConfig(ServerConfig &&) = default;
        ServerConfig &operator=(ServerConfig &&) = default;

        ~ServerConfig() = default;

        bool contains(const std::string &ip, const std::string &name,
                      const int port);
        void check_config();

        std::vector<VHostConfig> vhosts;
        std::map<std::string, Upstream> upstreams;
    };
} // namespace http
