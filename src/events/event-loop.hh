/**
 * \file events/event-loop.hh
 * \brief EventLoop declaration.
 */

#pragma once

#include <ev.h>
#include <iostream>

#include "error/not-implemented.hh"
#include "events/events.hh"
#include "misc/readiness/readiness.hh"

namespace http
{
    class EventWatcher;

    /**
     * \struct EventLoop
     * \brief Value object wrapping libev's ev_loop.
     *
     * Event loop design pattern.
     */
    struct EventLoop
    {
        /**
         * \brief Create a default EventLoop based on a default ev_loop.
         */
        EventLoop();

        /**
         * \brief Create an EventLoop from an existing ev_loop.
         *
         * \param loop ev_loop* custom ev_loop.
         */
        explicit EventLoop(struct ev_loop *);

        EventLoop(const EventLoop &) = default;
        EventLoop &operator=(const EventLoop &) = default;
        EventLoop(EventLoop &&) = default;
        EventLoop &operator=(EventLoop &&) = default;

        void start_timer(ev_timer *timer);
        void stop_timer(ev_timer *timer);

        /**
         * \brief Destroy the ev_loop.
         */
        ~EventLoop();

        /**
         * \brief Activate the given ev_io.
         *
         * Note that only activated watchers will receive events.
         *
         * \param watcher EventWatcher* to register in the loop.
         */
        void register_watcher(EventWatcher *watcher);

        /**
         * \brief Stop the given ev_io.
         *
         * \param watcher EventWatcher* to unregister in the loop.
         */
        void unregister_watcher(EventWatcher *watcher);

        /**
         * \brief Register SIGINT ev_signal.
         *
         * \param watcher ev_signal* to register in the loop.
         */
        void register_sigint_watcher(ev_signal *) const;

        /**
         * \brief Start ev_timer.
         *
         * \param watcher ev_timer* to register in the loop.
         */
        void register_timer_watcher(ev_timer *et) const
        {
            // This is just a hack for clang++ warning
            // change this body once you implement timeouts
            if (et)
                throw http::NotImplemented();
        }

        /**
         * \brief Stop ev_timer.
         *
         * \param watcher ev_timer* to unregister in the loop.
         */
        void unregister_timer_watcher(ev_timer *et) const
        {
            // This is just a hack for clang++ warning
            // change this body once you implement timeouts
            if (et)
                throw http::NotImplemented();
        }
        /**
         * \brief Start waiting for events.
         */
        void operator()() const
        {
            ev_run(EV_A, 0);
        }

        /**
         * \brief Libev's event loop.
         */
        struct ev_loop *loop;
    };

} // namespace http
