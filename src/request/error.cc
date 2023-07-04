#include "request/error.hh"

#include "request/types.hh"

namespace http::error
{
    Response bad_request(const Request &request)
    {
        return Response(request, STATUS_CODE::BAD_REQUEST, "Bad Request");
    }

    Response unauthorized(const Request &request, const std::string &realm)
    {
        auto auth_field = std::string{ "WWW-Authenticate: Basic realm=\"" };
        auth_field.append(realm);
        auth_field.push_back('\"');
        auto extra_field = std::vector<std::string>{ auth_field };
        return Response(request, STATUS_CODE::UNAUTHORIZED, "Unauthorized",
                        extra_field);
    }

    Response forbidden(const Request &request)
    {
        return Response(request, STATUS_CODE::FORBIDDEN, "Forbidden");
    }

    Response not_found(const Request &request)
    {
        return Response(request, STATUS_CODE::NOT_FOUND, "Not Found");
    }

    Response method_not_allowed(const Request &request)
    {
        return Response(request, STATUS_CODE::METHOD_NOT_ALLOWED,
                        "Method Not Allowed");
    }

    Response request_timeout(const std::string &reason)
    {
        std::string timeout_reason = "X-Timeout-Reason: ";
        timeout_reason.append(reason);
        std::vector<std::string> extra_fields = { timeout_reason };
        return Response(STATUS_CODE::REQUEST_TIMEOUT, "Request Timeout",
                        extra_fields);
    }

    Response proxy_authentication_required(const Request &request, shared_vhost)
    {
        // FIXME: Not implemented
        return Response(request, STATUS_CODE::PROXY_AUTHENTICATION_REQUIRED,
                        "Proxy Authentication Required");
    }

    Response payload_too_large(const Request &request)
    {
        return Response(request, STATUS_CODE::PAYLOAD_TOO_LARGE,
                        "Payload Too Large");
    }

    Response uri_too_long(const Request &request)
    {
        return Response(request, STATUS_CODE::URI_TOO_LONG, "URI Too Long");
    }

    Response upgrade_required(const Request &request)
    {
        return Response(request, STATUS_CODE::UPGRADE_REQUIRED,
                        "Upgrade Required");
    }

    Response header_fields_too_large(const Request &request)
    {
        return Response(request, STATUS_CODE::HEADER_FIELDS_TOO_LARGE,
                        "Header Fields Too Large");
    }

    Response internal_server_error(const Request &request)
    {
        return Response(request, STATUS_CODE::INTERNAL_SERVER_ERROR,
                        "Internal Server Error");
    }

    Response not_implemented(const Request &request)
    {
        return Response(request, STATUS_CODE::NOT_IMPLEMENTED,
                        "Not Implemented");
    }

    Response bad_gateway()
    {
        return Response(STATUS_CODE::BAD_GATEWAY, "Bad Gateway");
    }

    Response service_unavailable(const Request &request)
    {
        return Response(request, STATUS_CODE::SERVICE_UNAVAILABLE,
                        "Service Unavailable");
    }

    Response gateway_timeout()
    {
        return Response{ STATUS_CODE::GATEWAY_TIMEOUT, "Gateway Timeout" };
    }

    Response http_version_not_supported(const Request &request)
    {
        return Response(request, STATUS_CODE::HTTP_VERSION_NOT_SUPPORTED,
                        "HTTP Version Not Supported");
    }
} // namespace http::error
