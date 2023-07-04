#include "request/request.hh"

#include <algorithm>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

#include "misc/exceptions.hh"

namespace http
{
    void Request::parse_method(const std::string &token)
    {
        if (token.compare("GET") == 0)
            this->method = GET;
        else if (token.compare("HEAD") == 0)
            this->method = HEAD;
        else if (token.compare("POST") == 0)
            this->method = POST;
        else
            this->method = NOT_ALLOW;
    }

    void Request::parse_version(const std::string &token, shared_socket sock)
    {
        this->version = token;
        if (version == "HTTP/0.9" || version == "HTTP/1.0")
            this->upgrade_required = true;
        else if (version != "HTTP/1.1")
        {
            this->version = "HTTP/1.1";
            throw BadRequestException(
                *this, sock, "bad request: http version not in the format X.X");
        }
    }

    void Request::parse_request_line(const std::string &line,
                                     shared_socket sock)
    {
        auto sstream = std::stringstream(line);
        auto token = std::string{};
        std::getline(sstream, token, ' ');
        auto request_method = token;
        std::getline(sstream, token, ' ');
        this->target = token;
        std::getline(sstream, token, ' ');
        this->parse_version(token, sock);
        this->parse_method(request_method);
    }

    static void trim_string(std::string &str)
    {
        str.erase(0, str.find_first_not_of(" \t"));
        str.erase(str.find_last_not_of(" \t") + 1);
    }

    void Request::parse_header_line(const std::string &line, shared_socket sock)
    {
        if (line.find(':') == std::string::npos)
            throw BadRequestException(*this, sock, "bad request: no colon");
        auto iss = std::istringstream(line);
        auto field = std::string{};
        std::getline(iss, field, ':');
        if (field.find(' ') != std::string::npos)
            throw BadRequestException(*this, sock,
                                      "bad request: space in field");
        auto value = std::string{};
        std::getline(iss, value);
        trim_string(value);
        this->header_fields.insert(std::make_pair(field, value));
    }

    static void check_ends_with_CR(const Request &request, std::string &line,
                                   shared_socket sock)
    {
        if (line.back() != '\r')
            throw BadRequestException(request, sock,
                                      "bad request: no carriage return");
        line.pop_back();
    }

    void Request::parse_header(const std::string &header, shared_socket sock)
    {
        auto iss = std::istringstream(header);
        auto line = std::string{};
        std::getline(iss, line, '\n');
        check_ends_with_CR(*this, line, sock);
        this->parse_request_line(line, sock);
        while (std::getline(iss, line, '\n'))
        {
            check_ends_with_CR(*this, line, sock);
            this->parse_header_line(line, sock);
        }
        parse_connection();
        auto cl = header_fields.find("Content-Length");
        auto regex = std::regex("0|[1-9][0-9]*");
        if (cl != header_fields.end() && !std::regex_match(cl->second, regex))
            throw BadRequestException(*this, sock,
                                      "bad request: Content-Length not valid");
        if (this->method == NOT_ALLOW)
            throw MethodNotAllowedException(sock, *this);
        if (this->upgrade_required)
            throw UpgradeRequiredException(sock, *this);
    }

    void Request::parse_connection()
    {
        auto value_l = header_fields.find("Connection");
        if (value_l != header_fields.end())
        {
            std::cout << value_l->second << "\n";
            std::stringstream value_line(value_l->second);
            std::string token;
            while (std::getline(value_line, token, ','))
            {
                token.erase(remove_if(token.begin(), token.end(), isspace),
                            token.end());
                if (token == "close")
                {
                    std::cout << "Connection is " << token << "\n";
                    this->connect = http::connection::CLOSE;
                    break;
                }
            }
        }
    }

    std::optional<std::string> Request::at(const std::string &field)
    {
        try
        {
            return this->header_fields.at(field);
        }
        catch (...)
        {
            return std::nullopt;
        }
    }
} // namespace http
