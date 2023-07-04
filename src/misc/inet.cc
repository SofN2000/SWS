#include "misc/inet.hh"

#include <arpa/inet.h>

namespace http
{
    bool is_ipv6_addr_no_brackets(const std::string &ip)
    {
        char buffer[INET6_ADDRSTRLEN];
        return inet_pton(AF_INET6, ip.c_str(), buffer) == 1;
    }
    bool is_ipv6_addr(const std::string &ip)
    {
        if (*ip.begin() == '[' && *(ip.end() - 1) == ']')
        {
            char buffer[INET6_ADDRSTRLEN];
            auto ipv6 = ip.substr(1, ip.size() - 2);
            return inet_pton(AF_INET6, ipv6.c_str(), buffer) == 1;
        }
        return false;
    }

    bool is_ipv4_addr(const std::string &ip)
    {
        char buffer[INET_ADDRSTRLEN];
        return inet_pton(AF_INET, ip.c_str(), buffer) == 1;
    }

    bool is_ip_addr(const std::string &ip)
    {
        return is_ipv6_addr(ip) || is_ipv4_addr(ip);
    }

    bool ips_eq(const std::string &ip1, const std::string &ip2)
    {
        if (ip1.empty())
            return ip2.empty();
        if (ip1 == "0.0.0.0")
            return is_ipv4_addr(ip2);
        if (ip2 == "0.0.0.0")
            return is_ipv4_addr(ip1);
        if (ip1 == "::")
            return is_ipv6_addr(ip2);
        if (ip2 == "::")
            return is_ipv6_addr(ip1);
        return ip1 == ip2;
    }

} // namespace http
