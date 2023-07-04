/**
 * \file vhost/vhost-static-file.hh
 * \brief VHostStaticFile declaration.
 */

#pragma once

#include "config/config.hh"
#include "request/request.hh"
#include "request/response.hh"
#include "vhost/connection.hh"
#include "vhost/vhost-factory.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class VHostStaticFile
     * \brief VHost serving static files.
     */
    class VHostStaticFile : public VHost
    {
    public:
        friend class VHostFactory;
        virtual ~VHostStaticFile() = default;

    private:
        /**
         * \brief Constructor called by the factory.
         *
         * \param config VHostConfig virtual host configuration.
         */
        explicit VHostStaticFile(const VHostConfig &vhost_config)
            : VHost(vhost_config)
        {}

        /**
         * \brief Prepend vhost root and append default_file if target
         * is directory
         *
         * \param request Request
         */
        void define_target_resource(Request &request) const;

        /**
         * \brief Send response if VHost uses basic authentication
         * is directory
         *
         * \param request Request
         * \param sock shared_socket
         */
        void respond_basic(Request &request, shared_socket sock) final;

    public:
        /**
         * \brief Send response.
         *
         * \param req Request.
         * \param conn Connection. std::shared_ptr<Connection>
         *
         * Note that these iterators will only be useful starting from SRPS.
         */
        void respond(Request &request, shared_socket sock) final;
    };
} // namespace http
