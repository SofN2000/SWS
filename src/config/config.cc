#include "config.hh"

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "events/register.hh"
#include "misc/file_to_string.hh"
#include "misc/inet.hh"
#include "misc/json.hh"

namespace http
{
    LoadBalancingMethod lb_method_of_string(const std::string &method)
    {
        if (method == "round-robin")
            return LoadBalancingMethod::ROUND_ROBIN;
        if (method == "failover")
            return LoadBalancingMethod::FAILOVER;
        if (method == "fail-robin")
            return LoadBalancingMethod::FAIL_ROBIN;
        throw std::runtime_error("Unknown load balancing method");
    }

    static void check_vhost_config(const VHostConfig &vhost_config)
    {
        if (vhost_config.port < 0 || vhost_config.port > 65535
            || (!is_ipv6_addr_no_brackets(vhost_config.ip)
                && !is_ipv4_addr(vhost_config.ip)))
            throw std::runtime_error("bad ip or port in vhost config");
    }

    void ServerConfig::check_config()
    {
        for (const auto &vhost_config : vhosts)
            check_vhost_config(vhost_config);
        if (event_register.throughput_val.has_value()
            ^ event_register.throughput_time.has_value())
            throw std::runtime_error(
                "throughput_val and throughput_time are co-dependant");
    }

    bool ServerConfig::contains(const std::string &ip, const std::string &name,
                                const int port)
    {
        auto found =
            std::find_if(vhosts.begin(), vhosts.end(),
                         [&ip, &name, port](const VHostConfig &config) {
                             if (!ips_eq(ip, config.ip))
                                 return false;
                             if (!name.empty() && name != config.server_name)
                                 return false;
                             return port == config.port;
                         });
        return found != vhosts.end();
    }

    bool VHostConfig::ssl_is_valid() const
    {
        if (this->ssl_key == "")
            return this->ssl_cert == "";
        if (this->ssl_cert == "")
            return false;
        return access(this->ssl_key.c_str(), R_OK) != -1
            && access(this->ssl_cert.c_str(), R_OK) != -1;
    }

    bool VHostConfig::auth_basic_is_valid() const
    {
        if (this->auth_basic == "")
            return this->auth_basic_users.size() == 0;
        return this->auth_basic_users.size() != 0;
    }

    ServerConfig::ServerConfig(const std::string &path)
        : vhosts()
        , upstreams()
    {
        std::cout << "$ parse_configuration\n";
        auto json_content = file_to_string(path);
        auto parser = json::parse(json_content);
        auto vhosts_json = parser.at("vhosts");
        auto default_vhost = false;
        if (parser.contains("upstreams"))
        {
            auto upstreams_json = parser.at("upstreams");
            for (const auto &key_val : upstreams_json.items())
            {
                auto upstream_name = key_val.key();
                auto upstream_obj = Upstream(key_val.value());
                upstreams.insert(std::make_pair(upstream_name, upstream_obj));
            }
        }
        if (parser.contains("timeout"))
        {
            auto timeout_json = parser.at("timeout");
            if (timeout_json.contains("keep_alive"))
            {
                event_register.keep_alive = timeout_json.at("keep_alive");
                if (event_register.keep_alive.value() < 0)
                    throw std::invalid_argument(
                        "keep-alive timeout must be positive float");
            }
            if (timeout_json.contains("transaction"))
            {
                event_register.transaction = timeout_json.at("transaction");
                if (event_register.transaction.value() < 0)
                    throw std::invalid_argument(
                        "transaction timeout must be positive float");
            }
            if (timeout_json.contains("throughput_val"))
            {
                event_register.throughput_val =
                    timeout_json.at("throughput_val");
                if (event_register.throughput_val.value() < 0)
                    throw std::invalid_argument(
                        "throughput val must be positive integer");
            }
            if (timeout_json.contains("throughput_time"))
            {
                event_register.throughput_time =
                    timeout_json.at("throughput_time");
                if (event_register.throughput_time.value() < 0)
                    throw std::invalid_argument(
                        "throughput timeout must be positive float");
            }
        }
        for (const auto &vhost : vhosts_json)
        {
            auto config = VHostConfig(vhost);
            if (!config.ssl_is_valid())
                throw std::invalid_argument("ssl_key or ssl_cert missing.");
            if (!config.auth_basic_is_valid())
                throw std::invalid_argument(
                    "auth_basic auth_basic_users missing.");
            if (config.is_default)
            {
                if (default_vhost)
                    throw std::invalid_argument("Too many default vhosts.");
                default_vhost = true;
            }
            if (this->contains(config.ip, config.server_name, config.port))
                throw std::invalid_argument("VHosts not differentiable.");
            if (config.proxy_pass_opt.has_value())
            {
                auto upstream_name = config.proxy_pass_opt.value().upstream;
                if (!upstream_name.empty()
                    && upstreams.find(upstream_name) == upstreams.end())
                    throw std::invalid_argument("ProxyPass upstream unknown.");
            }
            this->vhosts.push_back(config);
        }
        this->check_config();
    }

} // namespace http
