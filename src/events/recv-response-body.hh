/**
 * \file events/recv-response-body.hh
 * \brief RecvResponseBodyEW declaration.
 */
#pragma once

#include "events/events.hh"
#include "request/response.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class RecvResponseBodyEW
     * \brief Workflow for listener socket.
     */
    class RecvResponseBodyEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a RecvBodyEW from listening socket.
         */
        explicit RecvResponseBodyEW(shared_socket client_socket,
                                    shared_socket backend_socket,
                                    shared_vhost vhost, Response response)
            : EventWatcher(backend_socket->fd_get()->fd_, EV_READ)
            , client_sock_(client_socket)
            , backend_sock_(backend_socket)
            , vhost_(vhost)
            , port_(0)
            , response_(response)
        {
            std::cout << ">>> RecvResponseBodyEW\n";
            this->sock_for_proxy_transaction_ = this->client_sock_;
            vhost->transaction_timer.data = reinterpret_cast<void *>(this);
        }

        /**
         * \brief Start or resume receiving response.
         */
        void operator()() final;

    private:
        /**
         * \brief check if the body is complete
         **/
        bool is_body_complete();
        /**
         * \brief RecvResponse client's socket.
         */
        shared_socket client_sock_;

        /**
         * \brief RecvResponse backend's socket.
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
         * \brief Struc representing a response.
         */
        struct Response response_;

        /**
         *\brief Check and stop the transaction timer
         */
        void check_and_stop_timer();
    };
} // namespace http
