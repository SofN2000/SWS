#include "events/register.hh"

#include <exception>

#include "events/event-loop.hh"

namespace http
{
    EventWatcherRegistry event_register;

    std::optional<std::shared_ptr<EventWatcher>>
    EventWatcherRegistry::at(EventWatcher *key)
    {
        try
        {
            return this->events_.at(key);
        }
        catch (const std::out_of_range &e)
        {
            return std::nullopt;
        }
    }

    void EventWatcherRegistry::run(char **argv)
    {
        misc::announce_spider_readiness(argv[0]);
        (loop_)();
    }

    void EventWatcherRegistry::unregister_ew(EventWatcher *watcher)
    {
        if (this->at(watcher) != std::nullopt
            && this->events_.erase(watcher) == 1)
            loop_.unregister_watcher(watcher);
    }

    void EventWatcherRegistry::start_timer(ev_timer *timer)
    {
        std::cout << "$ EventWatcherRegistry::start_timer\n";
        this->loop_.start_timer(timer);
    }

    void EventWatcherRegistry::stop_timer(ev_timer *timer)
    {
        std::cout << "$ EventWatcherRegistry::stop_timer" << std::endl;
        this->loop_.stop_timer(timer);
    }
} // namespace http
