#include "events/event-loop.hh"

#include <iostream>

#include "events/events.hh"

namespace http
{
    EventLoop::EventLoop()
    {
        std::cout << ">>> EventLoop\n";
        this->loop = EV_DEFAULT;
    }

    EventLoop::EventLoop(struct ev_loop *ev_loop)
    {
        std::cout << "=== EventLoop\n";
        this->loop = ev_loop;
    }

    EventLoop::~EventLoop()
    {
        std::cout << "<<< EventLoop\n";
        ev_loop_destroy(EV_A);
    }

    void EventLoop::register_watcher(EventWatcher *watcher)
    {
        std::cout << "$ register_watcher\n";
        ev_io_start(EV_A, &watcher->watcher_get());
    }

    void EventLoop::unregister_watcher(EventWatcher *watcher)
    {
        std::cout << "$ unregister_watcher\n";
        ev_io_stop(EV_A, &watcher->watcher_get());
    }

    void EventLoop::start_timer(ev_timer *timer)
    {
        std::cout << "$ EventLoop::start_timer\n";
        ev_timer_start(EV_A_ timer);
    }

    void EventLoop::stop_timer(ev_timer *timer)
    {
        std::cout << "$ EventLoop::stop_timer" << std::endl;
        ev_timer_stop(EV_A_ timer);
    }

} // namespace http
