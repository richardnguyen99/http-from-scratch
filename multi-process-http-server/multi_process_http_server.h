#ifndef __HFS_MULTI_PROCESS_HTTP_SERVER_H__
#define __HFS_MULTI_PROCESS_HTTP_SERVER_H__ 1

#include <http_core.h>
#include <http_server.h>

namespace hfs
{
class multi_process_http_server : public http_server_base
{
public:
    multi_process_http_server();
    ~multi_process_http_server();

    void
    start() override;

    void
    listen(int port, int backlog = 128, int optval = 0) override;

    void
    register_handler(
        const std::string &path, const std::string &method,
        std::function<void()> handler
    ) override;
};
} // namespace hfs

#endif // __HFS_MULTI_PROCESS_HTTP_SERVER_H__
