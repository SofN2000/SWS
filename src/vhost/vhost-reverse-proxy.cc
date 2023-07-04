#include "vhost-reverse-proxy.hh"

#include "events/register.hh"
#include "events/send-request.hh"
#include "events/send-response.hh"

namespace http
{
    void VHostReverseProxy::respond(Request &request,
                                    shared_socket client_sock_)
    {
        auto vhost_rev =
            shared_reverse_vhost(new VHostReverseProxy(this->conf_));
        if (this->ip_port_opt_.has_value())
        {
            event_register.register_event<SendRequestEW>(
                client_sock_, request, vhost_rev, this->ip_port_opt_.value());
        }
        else
        {
            auto host = this->load_balancer_->GetNextHost();
            auto ip_port = std::make_pair(host.ip, host.port);
            event_register.register_event<SendRequestEW>(client_sock_, request,
                                                         vhost_rev, ip_port);
        }
    }

    void VHostReverseProxy::process_response(Response &)
    {
        // XXX why here?
    }

    void VHostReverseProxy::respond_basic(Request &, shared_socket)
    {
        return;
    }
} // namespace http
