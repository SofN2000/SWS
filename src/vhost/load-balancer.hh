/**
 * \file vhost/load_balancer.hh
 * \brief LoadBalancer declaration.
 */

#pragma once

#include "config/config.hh"

namespace http
{
    class LoadBalancer
    {
    public:
        explicit LoadBalancer(std::vector<HostConfig> list)
            : list_load_b_(list)
        {}

        virtual HostConfig GetNextHost() = 0;

    protected:
        std::vector<HostConfig> list_load_b_;
    };

    class RoundRobin : public LoadBalancer
    {
    public:
        explicit RoundRobin(std::vector<HostConfig> list)
            : LoadBalancer(list){};

        HostConfig GetNextHost() final;

    private:
        size_t index = 0;
        size_t count = 0;
    };

    class FailOver : public LoadBalancer
    {
    public:
        explicit FailOver(std::vector<HostConfig> list)
            : LoadBalancer(list){};

        HostConfig GetNextHost() final;
    };

    class FailRobin : public LoadBalancer
    {
    public:
        explicit FailRobin(std::vector<HostConfig> list)
            : LoadBalancer(list){};

        HostConfig GetNextHost() final;
    };

    class BatmanRobin : public LoadBalancer
    {
    public:
        explicit BatmanRobin(std::vector<HostConfig> list)
            : LoadBalancer(list){};

        HostConfig GetNextHost() final;
    };

    using shared_load_balancer = std::shared_ptr<LoadBalancer>;
} // namespace http
