#ifndef VNC_BASE_H
#define VNC_BASE_H

#include <string>
#include <vector>
#include <queue>
#include <functional>

#include "../base/frame_base.h"

namespace vnc
{

const std::uint32_t default_port = 5900;
const std::string default_password = "";
const std::uint32_t default_bpp = 24;

using frame_size_t = base::frame_size_t;

typedef std::vector<std::uint8_t> frame_data_t;

struct vnc_server_config_t
{
    std::string     host;
    std::string     password;
    std::uint32_t   port;

    vnc_server_config_t(const std::string& host
                        , const std::string& password = default_password
                        , std::uint32_t port = default_port);

    std::string uri() const;


};

struct vnc_config_t
{

};

struct frame_t
{
    frame_size_t    frame_size;
    frame_data_t    frame_data;
    std::uint32_t   bpp;

    frame_t(const frame_size_t frame_size = frame_size_t()
            , std::uint32_t bpp = default_bpp);

    std::size_t realloc();

};

typedef std::queue<frame_t> frame_queue_t;

typedef std::function<bool(frame_t&& frame)> frame_handler_t;

}

#endif // VNC_BASE_H
