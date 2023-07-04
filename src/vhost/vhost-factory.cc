#include "vhost/vhost-factory.hh"

#include "vhost/vhost-reverse-proxy.hh"
#include "vhost/vhost-static-file.hh"

namespace http
{
    shared_vhost VHostFactory::Create(VHostConfig vhost_config)
    {
        if (vhost_config.proxy_pass_opt.has_value())
        {
            auto vhost_ptr = new VHostReverseProxy(vhost_config);
            auto vhost_shared_ptr =
                std::shared_ptr<VHostReverseProxy>(vhost_ptr);
            return vhost_shared_ptr;
        }
        auto vhost_ptr = new VHostStaticFile(vhost_config);
        auto vhost_shared_ptr = std::shared_ptr<VHostStaticFile>(vhost_ptr);
        return vhost_shared_ptr;
    }

} // namespace http
