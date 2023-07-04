/**
 * \file request/error.hh
 * \brief http::error::<error> declarations.
 */

#pragma once

#include "request/response.hh"
#include "request/types.hh"
#include "vhost/vhost.hh"

namespace http::error
{
    /**
     * \brief Create an error response from a request.
     */
    Response bad_request(const Request &request);
    Response unauthorized(const Request &request, const std::string &realm);
    Response forbidden(const Request &request);
    Response not_found(const Request &request);
    Response method_not_allowed(const Request &request);
    Response proxy_authentication_required(const Request &request,
                                           shared_vhost vhost);
    Response request_timeout(const std::string &reason);
    Response payload_too_large(const Request &request);
    Response uri_too_long(const Request &request);
    Response upgrade_required(const Request &request);
    Response header_fields_too_large(const Request &request);
    Response internal_server_error(const Request &request);
    Response not_implemented(const Request &request);
    Response bad_gateway();
    Response service_unavailable(const Request &request);
    Response gateway_timeout();
    Response http_version_not_supported(const Request &request);
} // namespace http::error
