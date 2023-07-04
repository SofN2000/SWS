/**
 * \file events/recv-response-header.hh
 * \brief RecvResponseHeaderEW declaration.
 */
#pragma once

#include "events/events.hh"
#include "request/request.hh"
#include "request/response.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class RecvResponseHeaderEW
     * \brief Workflow for listener socket.
     */
    class RecvResponseHeaderEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a RecvRepsonseHeaderEW from listening socket.
         */
        explicit RecvResponseHeaderEW(shared_socket client_socket,
                                      shared_socket backend_socket,
                                      shared_vhost vhost,
                                      enum Request::method method)
            : EventWatcher(backend_socket->fd_get()->fd_, EV_READ)
            , client_sock_(client_socket)
            , backend_sock_(backend_socket)
            , vhost_(vhost)
            , port_(0)
            , method_(method)
        {
            std::cout << ">>> RecvResponseHeaderEW\n";
            this->sock_for_proxy_transaction_ = this->client_sock_;
            vhost->transaction_timer.data = reinterpret_cast<void *>(this);
        }

        /**
         * \brief Start or resume receiving response from backend.
         */
        void operator()() final;

    private:
        /**
         * \brief parse header
         */
        void parse_header();

        /**
         * \brief returns true if header is complete and remove body from header
         */
        bool is_complete();

        /**
         * \brief client socket.
         */
        shared_socket client_sock_;
        /**
         * \brief backend socket.
         */
        shared_socket backend_sock_;
        /**
         * \brief related reverse proxy vhost.
         */
        shared_vhost vhost_;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
        /**
         * \brief String representing a header
         */
        std::string header_;
        /**
         * \brief Struct representing a response
         */
        struct Response response_;

        /**
         * \brief Store the method of the client's request
         * to know if a body should be called.
         */
        enum Request::method method_;

        /**
         *\brief Process the response based on the set and remove_header
         *\headers from vhost
         */
        Response process_response(Response response);

        /**
         *\brief Check and stop the transaction timer
         */
        void check_and_stop_timer();
    };
} // namespace http
