/**
 * \file events/send-response.hh
 * \brief SendResponseEW declaration.
 */

#pragma once

#include <arpa/inet.h>
#include <iostream>

#include "events/events.hh"
#include "events/register.hh"
#include "misc/socket.hh"
#include "request/request.hh"
#include "request/response.hh"
#include "socket/socket.hh"

namespace http
{
    /**
     * \class SendResponseEW
     * \brief Workflow for listener socket.
     */
    class SendResponseEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a SendResponseEW from listening socket.
         */
        explicit SendResponseEW(shared_socket socket, Response response)
            : EventWatcher(socket->fd_get()->fd_, EV_WRITE)
            , sock_(socket)
            , port_(0)
            , response_(response)
        {
            std::cout << ">>> SendResponseEW\n";
        }

        /**
         * \brief Start or resume sending response.
         */
        void operator()() final;

    private:
        /**
         * \brief SendResponse socket.
         */
        shared_socket sock_;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
        /**
         * \brief Struct representing a response.
         */
        struct Response response_;
    };
} // namespace http
