/**
 * \file request/request.hh
 * \brief Request declaration.
 */

#pragma once

#include <string>
#include <unordered_map>

#include "request/types.hh"
#include "socket/socket.hh"

namespace http
{
    enum connection
    {
        CLOSE,
        KEEPALIVE,
    };

    /**
     * \struct Request
     * \brief Value object representing a request.
     */
    struct Request
    {
    public:
        enum method
        {
            GET,
            HEAD,
            POST,
            NOT_ALLOW
        };

        Request() = default;

        Request(const Request &) = default;
        Request &operator=(const Request &) = default;
        Request(Request &&) = default;
        Request &operator=(Request &&) = default;
        ~Request() = default;

        std::optional<std::string> at(const std::string &field);

        void parse_header(const std::string &str, shared_socket sock);

        method method;
        bool upgrade_required = false;
        std::string version;
        std::string target;
        std::unordered_map<std::string, std::string> header_fields;
        std::string body;
        connection connect = http::connection::KEEPALIVE;
        bool auto_index = false;
        std::string vhost_root;

    private:
        void parse_request_line(const std::string &line, shared_socket sock);
        void parse_method(const std::string &token);
        void parse_version(const std::string &token, shared_socket sock);
        void parse_header_line(const std::string &line, shared_socket sock);
        void parse_connection();
    };
} // namespace http
