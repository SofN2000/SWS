#pragma once

#include <string>

namespace http
{
    bool is_ipv6_addr_no_brackets(const std::string &ip);
    bool is_ipv6_addr(const std::string &ip);
    bool is_ipv4_addr(const std::string &ip);
    bool is_ip_addr(const std::string &ip);
    bool ips_eq(const std::string &ip1, const std::string &ip2);
} // namespace http
