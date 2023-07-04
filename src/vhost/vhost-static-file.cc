#include "vhost/vhost-static-file.hh"

#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#include "events/register.hh"
#include "events/send-response.hh"
#include "misc/openssl/base64.hh"
#include "misc/uri.hh"
#include "request/error.hh"
#include "request/response.hh"

#define URI_MAX_LEN 255

namespace http
{
    void VHostStaticFile::define_target_resource(Request &request) const
    {
        if (request.target.back() == '/')
            request.target.append(this->conf_.default_file);
        process_target_resource(request.target);
        request.target.insert(0, this->conf_.root);
    }

    static void reply401(const Request &request, const VHostConfig &config,
                         shared_socket sock)
    {
        auto realm = config.auth_basic;
        event_register.register_event<SendResponseEW>(
            sock, error::unauthorized(request, realm));
    }

    void VHostStaticFile::respond_basic(Request &request, shared_socket sock)
    {
        auto auth_field = request.header_fields.find("Authorization");
        if (auth_field == request.header_fields.end())
            return reply401(request, this->conf_, sock);
        auto iss = std::istringstream{ auth_field->second };
        auto token = std::string{};
        std::getline(iss, token, ' ');
        if (token != "Basic")
            return reply401(request, this->conf_, sock);
        std::getline(iss, token);
        auto decoded = ssl::base64_decode(token);
        auto begin = this->conf_.auth_basic_users.cbegin();
        auto end = this->conf_.auth_basic_users.cend();
        if (std::find(begin, end, decoded) == end)
            return reply401(request, this->conf_, sock);
        if (request.connect == connection::CLOSE)
            this->client_sock_set(-1);
        event_register.register_event<SendResponseEW>(sock, Response(request));
    }

    void VHostStaticFile::respond(Request &request, shared_socket sock)
    {
        request.auto_index = this->conf_get().auto_index;
        request.vhost_root = this->conf_get().root;
        this->define_target_resource(request);
        if (request.target.length() > URI_MAX_LEN)
        {
            event_register.register_event<SendResponseEW>(
                sock, error::uri_too_long(request));
        }
        else if (access(request.target.c_str(), F_OK) == -1)
        {
            event_register.register_event<SendResponseEW>(
                sock, error::not_found(request));
        }
        else if (access(request.target.c_str(), R_OK) == -1)
        {
            event_register.register_event<SendResponseEW>(
                sock, error::forbidden(request));
        }
        else if (!this->conf_.auth_basic.empty())
        {
            this->respond_basic(request, sock);
        }
        else
        {
            if (request.connect == connection::CLOSE)
                this->client_sock_set(-1);
            event_register.register_event<SendResponseEW>(sock,
                                                          Response(request));
        }
    }
} // namespace http
