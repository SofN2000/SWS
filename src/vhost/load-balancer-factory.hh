#pragma once

#include "vhost/load-balancer.hh"

namespace http
{
    class LoadBalancerFactory
    {
    public:
        static shared_load_balancer Create(std::vector<HostConfig> list,
                                           const LoadBalancingMethod method);
    };
} // namespace http
