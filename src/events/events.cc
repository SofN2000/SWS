#include "events/events.hh"

#include "events/register.hh"
#include "events/send-response.hh"
#include "request/error.hh"

namespace http
{
    EventWatcher::EventWatcher(int fd, int flags)
    {
        ev_io_init(&watcher_, EventWatcher::event_callback, fd, flags);
        watcher_.data = reinterpret_cast<void *>(this);
    }

    void EventWatcher::event_callback(struct ev_loop *, ev_io *w, int)
    {
        auto ew = reinterpret_cast<EventWatcher *>(w->data);
        auto shared_ew = event_register.at(ew).value();
        (*shared_ew)();
    }

    static auto *recvheader_of_ev_timer(ev_timer *w)
    {
        auto ew = reinterpret_cast<EventWatcher *>(w->data);
        auto ew_opt = event_register.at(ew);
        auto shared_ew = ew_opt.value();
        auto rv = dynamic_cast<EventWatcher *>(shared_ew.get());
        return rv;
    }

    void EventWatcher::proxy_transaction_core()
    {
        std::cout << "CB: EventWatcher::proxy_transaction_callback_core\n";
        event_register.register_event<SendResponseEW>(
            this->sock_for_proxy_transaction_, error::gateway_timeout());
        event_register.unregister_ew(this);
    }

    void EventWatcher::proxy_transaction_cb(EV_P_ ev_timer *w, int)
    {
        std::cout << "CB: EventWatcher::proxy_transaction_callback\n";
        auto rv = recvheader_of_ev_timer(w);
        rv->proxy_transaction_core();
    }

} // namespace http
