/**
 * \file events/recvrequest.hh
 * \brief RecvHeaderEW declaration.
 */

#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <string>

#include "events/event-loop.hh"
#include "events/events.hh"
#include "events/recvbody.hh"
#include "events/register.hh"
#include "misc/socket.hh"
#include "request/request.hh"
#include "socket/socket.hh"
#include "vhost/dispatcher.hh"

namespace http
{
    /**
     * \class RecvHeaderEW
     * \brief Workflow for listener socket.
     */
    class RecvHeaderEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a RecvHeaderEW from listening socket.
         */
        explicit RecvHeaderEW(shared_socket socket)
            : EventWatcher(socket->fd_get()->fd_, EV_READ)
            , sock_(socket)
            , port_(0)
            , transaction_timer_{}
            , keep_alive_timer_{}
            , throughput_timer_{}
        {
            if (event_register.keep_alive.has_value())
            {
                auto delay = event_register.keep_alive.value();
                ev_timer_init(&keep_alive_timer_, keep_alive_callback, delay,
                              0);
                keep_alive_timer_.data = reinterpret_cast<void *>(this);
                event_register.start_timer(&this->keep_alive_timer_);
            }
            std::cout << ">>> RecvHeaderEW\n";
        }

        /**
         * \brief Start or resume receiving request.
         */
        void operator()() final;

    private:
        /**
         * \brief parse header
         */
        void parse_header();

        /**
         * \brief returns true if header is complete
         */
        bool is_complete(const std::string &header) const;

        void transition_callback_core();
        static void transaction_callback(EV_P_ ev_timer *w, int);

        void keep_alive_callback_core();
        static void keep_alive_callback(EV_P_ ev_timer *w, int);

        void throughput_callback_core();
        static void throughput_callback(EV_P_ ev_timer *w, int);

        /**
         * \brief RecvRequest socket.
         */
        shared_socket sock_;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
        /**
         * \brief String representing a header
         */
        std::string header_;
        /**
         * \brief Struc representing a request
         */
        struct Request request_;

        ev_timer transaction_timer_;
        ev_timer keep_alive_timer_;
        ev_timer throughput_timer_;
    };
} // namespace http
