/**
 * \file request/response.hh
 * \brief Response declaration.
 */

#pragma once

#include "request/request.hh"
#include "request/response.hh"
#include "request/types.hh"

namespace http
{
    /**
     * \struct Response
     * \brief Value object representing a response.
     */
    struct Response
    {
        Response(const Request &request);
        Response(const std::string header);
        Response(const STATUS_CODE &code, const std::string &reason_phrase);
        Response(const Request &request, const STATUS_CODE &code,
                 const std::string &reason_phrase);
        Response(const STATUS_CODE &code, const std::string &reason_phrase,
                 const std::vector<std::string> &extra_fields);
        Response(const Request &request, const STATUS_CODE &code,
                 const std::string &reason_phrase,
                 const std::vector<std::string> &extra_fields);

        Response() = default;
        Response(const Response &) = default;
        Response &operator=(const Response &) = default;
        Response(Response &&) = default;
        Response &operator=(Response &&) = default;
        ~Response() = default;

        void parse_status_line(const std::string &line);
        void parse_header_line(const std::string &line);
        std::string to_string() const;

        std::string version;
        STATUS_CODE status_code;
        std::string phrase;
        std::unordered_map<std::string, std::string> header_fields;
        std::string body;
        std::string str;
        connection connect = http::connection::CLOSE;
    };
} // namespace http
