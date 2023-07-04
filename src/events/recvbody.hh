/**
 * \file events/recvrequest.hh
 * \brief RecvBodyEW declaration.
 */

#pragma once

#include <arpa/inet.h>
#include <iostream>

#include "events/events.hh"
#include "events/register.hh"
#include "misc/socket.hh"
#include "request/request.hh"
#include "socket/socket.hh"
#include "vhost/dispatcher.hh"

namespace http
{
    /**
     * \class RecvBodyEW
     * \brief Workflow for listener socket.
     */
    class RecvBodyEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a RecvBodyEW from listening socket.
         */
        explicit RecvBodyEW(shared_socket socket, Request request)
            : EventWatcher(socket->fd_get()->fd_, EV_READ)
            , sock_(socket)
            , port_(0)
            , request_(request)
        {
            std::cout << ">>> RecvBodyEW\n";
        }

        /**
         * \brief Start or resume receiving request.
         */
        void operator()() final;

    private:
        /**
         * \brief RecvRequest socket.
         */
        shared_socket sock_;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
        /**
         * \brief Struc representing a request.
         */
        struct Request request_;
    };
} // namespace http
