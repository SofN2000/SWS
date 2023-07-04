#pragma once

#include "config/config.hh"

namespace http
{
    /**
     * \brief Run the event loop
     *
     * \param server_config the server config file.
     * \param argv argv
     */
    void run(const ServerConfig &server_config, char **argv);
} // namespace http
