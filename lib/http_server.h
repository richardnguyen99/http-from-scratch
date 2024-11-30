#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__ 1

#include "http_core.h"
#include "http_router.h"

#define HTTP_SERVER_USER_AGENT "http-from-scratch server"
namespace hfs
{

class http_server_base
{
public:
    http_server_base()          = default;
    virtual ~http_server_base() = default;

    virtual void
    start() = 0;

    virtual void
    listen(int port, int backlog = 128, int optval = 0) = 0;

    virtual void
    register_handler(
        const std::string &path, const std::string &method,
        hfs::http_router::route_handler_t handler
    ) = 0;

protected:
    struct addrinfo __hints;
    int __port;
    int __socket_flag;
    int __socket;
    int __backlog;
    hfs::http_router __router;
};
} // namespace hfs

#endif // __HTTP_SERVER_H__
