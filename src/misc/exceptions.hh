#pragma once

#include <exception>
#include <iostream>
#include <string>

#include "events/register.hh"
#include "events/send-response.hh"
#include "request/error.hh"
#include "request/request.hh"

namespace http
{
    class RequestException : std::exception
    {
    public:
        RequestException() = default;
        virtual const char *what() = 0;
    };

    class UpgradeRequiredException : public RequestException
    {
    public:
        UpgradeRequiredException(shared_socket sock, Request request)
        {
            event_register.register_event<SendResponseEW>(
                sock, error::upgrade_required(request));
        }
        const char *what() override
        {
            return "bad request: bad http version";
        };
    };

    class BadRequestException : public RequestException
    {
    public:
        BadRequestException(const Request &request, shared_socket sock,
                            std::string msg)
            : msg_{ msg }
        {
            event_register.register_event<SendResponseEW>(
                sock, error::bad_request(request));
        }
        const char *what() override
        {
            return msg_.c_str();
        };

    private:
        std::string msg_;
    };

    class MethodNotAllowedException : public RequestException
    {
    public:
        MethodNotAllowedException(shared_socket sock, Request request)
        {
            event_register.register_event<SendResponseEW>(
                sock, error::method_not_allowed(request));
        }
        const char *what() override
        {
            return "bad request: unknown method";
        };
    };

} // namespace http

std::ostream &operator<<(std::ostream &o, http::RequestException &e);
