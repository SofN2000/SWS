/**
 * \file vhost/vhost-factory.hh
 * \brief VHostFactory
 */

#pragma once

#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class VHostFactory
     * \brief Factory design pattern to create VHost.
     */
    class VHostFactory
    {
    public:
        /**
         * \brief Create a VHost object from a given VHostConfig.
         */
        static shared_vhost Create(VHostConfig vhost_config);
    };
} // namespace http
