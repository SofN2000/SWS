/**
 * \file events/send-response.hh
 * \brief SendRequestEW declaration.
 */
#pragma once

#include "config/config.hh"
#include "events/events.hh"
#include "events/register.hh"
#include "request/request.hh"
#include "socket/default-socket.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost-reverse-proxy.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class SendRequestEW
     * \brief Workflow for listener socket.
     */
    class SendRequestEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a SendRequestEW from listening socket.
         */
        explicit SendRequestEW(shared_socket socket, Request request,
                               shared_reverse_vhost vhost,
                               std::pair<std::string, int> ip_port)
            : EventWatcher(socket->fd_get()->fd_, EV_WRITE)
            , client_sock_(socket)
            , port_(0)
            , request_(request)
            , vhost_(vhost)
        {
            auto fd =
                create_server_socket(ip_port.first, ip_port.second)->fd_get();
            this->backend_sock_ = std::make_shared<DefaultSocket>(fd);
            str_from_request();
            if (vhost->conf_get().proxy_pass_opt.has_value())
            {
                if (vhost->conf_get()
                        .proxy_pass_opt.value()
                        .transaction_timeout.has_value())
                {
                    auto delay = vhost->conf_get()
                                     .proxy_pass_opt.value()
                                     .transaction_timeout.value();
                    ev_timer_init(&this->vhost_->transaction_timer,
                                  this->proxy_transaction_cb, delay, 0);
                    this->sock_for_proxy_transaction_ = this->client_sock_;
                    this->vhost_->transaction_timer.data =
                        reinterpret_cast<void *>(this);
                    event_register.start_timer(
                        &this->vhost_->transaction_timer);
                }
            }
            std::cout << ">>> SendRequestEW\n";
        }

        /**
         * \brief Start or resume sending request to backend services.
         */
        void operator()() final;

    private:
        /**
         * \brief SendRequest socket.
         */
        shared_socket client_sock_;

        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;

        /**
         * \brief Struct representing a request.
         */
        struct Request request_;

        /**
         * \brief String representing reconstructed request.
         */
        std::string str_;

        shared_reverse_vhost vhost_;

        shared_socket backend_sock_;

        void str_from_request();

        shared_socket create_server_socket(const std::string ip,
                                           const int port);

        Request process_request(Request request, std::string backend_ip);
    };
} // namespace http
