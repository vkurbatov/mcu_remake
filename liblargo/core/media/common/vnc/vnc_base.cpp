#include "vnc_base.h"

#include "media/common/base/string_base.h"

namespace vnc
{

vnc_server_config_t::vnc_server_config_t(const std::string &host
                                         , const std::string &password
                                         , uint32_t port)
    : host(host)
    , password(password)
    , port(port)
{

}

std::string vnc_server_config_t::uri() const
{
    std::string vnc_uri;

    vnc_uri.reserve(password.size() + host.size() + 32);

    vnc_uri.append("vnc://");

    if (!password.empty())
    {
        vnc_uri.append(password);
        vnc_uri.append("@");
    }

    vnc_uri.append(host);
    vnc_uri.append(":");
    vnc_uri.append(std::to_string(port));

    return vnc_uri;
}

frame_t::frame_t(const frame_size_t frame_size
                 , uint32_t bpp)
    : frame_size(frame_size)
    , bpp(bpp)
{
    realloc();
}

std::size_t frame_t::realloc()
{
    frame_data.resize((frame_size.size() * bpp) / 8);
}


}
