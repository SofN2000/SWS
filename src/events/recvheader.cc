#include "events/recvheader.hh"

#include "events/send-response.hh"
#include "misc/exceptions.hh"
#include "request/error.hh"

namespace http
{
    static bool ends_with(std::string const &value, std::string const &end)
    {
        if (end.size() > value.size())
            return false;
        return std::equal(end.rbegin(), end.rend(), value.rbegin());
    }

    bool RecvHeaderEW::is_complete(const std::string &header) const
    {
        std::cout << "$ is_complete" << '\n';
        return ends_with(header, "\r\n\r\n");
    }

    static void remove_trailing_crlf(std::string &line)
    {
        line = line.substr(0, line.size() - 2);
    }

    void RecvHeaderEW::parse_header()
    {
        std::cout << "$ parse_header\n";
        if (event_register.transaction.has_value())
            event_register.stop_timer(&this->transaction_timer_);
        if (event_register.throughput_val.has_value())
            event_register.stop_timer(&this->throughput_timer_);
        event_register.unregister_ew(this);
        try
        {
            remove_trailing_crlf(this->header_);
            this->request_.parse_header(this->header_, this->sock_);
            dispatcher.dispatch(this->request_, this->sock_);
        }
        catch (RequestException &e)
        {
            std::cout << e.what() << '\n';
        }
    }

    static RecvHeaderEW *recvheader_of_ev_timer(ev_timer *w)
    {
        auto ew = reinterpret_cast<EventWatcher *>(w->data);
        auto ew_opt = event_register.at(ew);
        auto shared_ew = ew_opt.value();
        auto rv = dynamic_cast<RecvHeaderEW *>(shared_ew.get());
        return rv;
    }

    void RecvHeaderEW::transition_callback_core()
    {
        std::cout << "CB: RecvHeaderEW::transaction_callback_core\n";
        event_register.register_event<SendResponseEW>(
            this->sock_, error::request_timeout("Transaction"));
        event_register.unregister_ew(this);
    }

    void RecvHeaderEW::transaction_callback(EV_P_ ev_timer *w, int)
    {
        std::cout << "CB: RecvHeaderEW::transaction_callback\n";
        auto rv = recvheader_of_ev_timer(w);
        rv->transition_callback_core();
    }

    void RecvHeaderEW::keep_alive_callback_core()
    {
        std::cout << "CB: RecvHeaderEW::keep_alive_callback_core\n";
        event_register.register_event<SendResponseEW>(
            this->sock_, error::request_timeout("Keep-Alive"));
        event_register.unregister_ew(this);
    }

    void RecvHeaderEW::keep_alive_callback(EV_P_ ev_timer *w, int)
    {
        std::cout << "CB: RecvHeaderEW::keep_alive_callback\n";
        auto rv = recvheader_of_ev_timer(w);
        rv->keep_alive_callback_core();
    }

    void RecvHeaderEW::throughput_callback_core()
    {
        std::cout << "CB: RecvHeaderEW::throughput_callback_core\n";
        if (this->header_.size() < event_register.throughput_val)
        {
            event_register.register_event<SendResponseEW>(
                this->sock_, error::request_timeout("Throughput"));
            event_register.unregister_ew(this);
        }
        else
            event_register.stop_timer(&this->throughput_timer_);
    }

    void RecvHeaderEW::throughput_callback(EV_P_ ev_timer *w, int)
    {
        std::cout << "CB: RecvHeaderEW::transaction_callback\n";
        auto rv = recvheader_of_ev_timer(w);
        rv->throughput_callback_core();
    }

    void RecvHeaderEW::operator()()
    {
        if (this->header_.empty())
        {
            if (event_register.keep_alive.has_value())
                event_register.stop_timer(&this->keep_alive_timer_);
            if (event_register.transaction.has_value())
            {
                auto delay = event_register.transaction.value();
                ev_timer_init(&this->transaction_timer_, transaction_callback,
                              delay, 0);
                this->transaction_timer_.data = reinterpret_cast<void *>(this);
                event_register.start_timer(&this->transaction_timer_);
            }
            if (event_register.throughput_val.has_value())
            {
                auto delay = event_register.throughput_time.value();
                ev_timer_init(&this->throughput_timer_, throughput_callback,
                              delay, 0);
                this->throughput_timer_.data = reinterpret_cast<void *>(this);
                event_register.start_timer(&this->throughput_timer_);
            }
        }
        std::cout << "CB: RecvHeaderEW\n";
        char buffer[DEFAULT_BUFFER_SIZE];
        try
        {
            auto read = sock_->recv(&buffer, DEFAULT_BUFFER_SIZE);
            if (read == 0)
            {
                std::cout << "CB: RecvHeaderEW: Client Disconnected\n";
                if (event_register.transaction.has_value())
                    event_register.stop_timer(&this->transaction_timer_);
                if (event_register.throughput_val.has_value())
                    event_register.stop_timer(&this->throughput_timer_);
                event_register.unregister_ew(this);
            }
            else
            {
                this->header_.append(buffer, read);
                std::cout << this->header_;
                if (this->is_complete(this->header_))
                    this->parse_header();
            }
        }
        catch (std::exception &e)
        {
            event_register.unregister_ew(this);
        }
    }
} // namespace http
