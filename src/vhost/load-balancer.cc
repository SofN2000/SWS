#include "load-balancer.hh"

namespace http
{
    HostConfig RoundRobin::GetNextHost()
    {
        if (count < list_load_b_[index].weight)
        {
            count++;
            return list_load_b_[index];
        }
        else
        {
            count = 1;
            index = (index + 1) % (list_load_b_.size());
            return list_load_b_[index];
        }
    }

    HostConfig FailOver::GetNextHost()
    {
        // FIXME
        return this->list_load_b_[0];
    }

    HostConfig FailRobin::GetNextHost()
    {
        // FIXME
        return this->list_load_b_[0];
    }
} // namespace http
