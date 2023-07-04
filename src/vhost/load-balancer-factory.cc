#include "load-balancer-factory.hh"

#include <iostream>

namespace http
{
    shared_load_balancer
    LoadBalancerFactory::Create(std::vector<HostConfig> list,
                                const LoadBalancingMethod method)
    {
        switch (method)
        {
        case LoadBalancingMethod::ROUND_ROBIN: {
            auto balancer_ptr = new RoundRobin(list);
            return std::shared_ptr<RoundRobin>(balancer_ptr);
        }
        case LoadBalancingMethod::FAILOVER: {
            auto balancer_ptr = new FailOver(list);
            return std::shared_ptr<FailOver>(balancer_ptr);
        }
        case LoadBalancingMethod::FAIL_ROBIN: {
            auto balancer_ptr = new FailRobin(list);
            return std::shared_ptr<FailRobin>(balancer_ptr);
        }
        default:
            std::cout << "It's not who I am underneath, but what I do that "
                         "defines me.\n";
            break;
        };
        throw std::runtime_error("You either die a here or live long enough to "
                                 "see yourself become the villain.\n");
    }
} // namespace http
