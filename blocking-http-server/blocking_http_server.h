#ifndef __HFS_BLOCKING_HTTP_SERVER_H__
#define __HFS_BLOCKING_HTTP_SERVER_H__ 1

#include <http_core.h>
#include <http_request.h>
#include <http_response.h>
#include <http_server.h>

namespace hfs
{
class blocking_http_server : public http_server_base
{
public:
    blocking_http_server();
    ~blocking_http_server();

    void
    start() override;

    void
    listen(int port, int backlog = 128, int optval = 1) override;

    void
    register_handler(
        const std::string &path, const std::string &method,
        http_router::route_handler_t handler
    ) override;

    void
    register_static_handler(const std::string &path) override;

    void
    register_error_handler(
        hfs::http_status_code_t status_code, const std::string &path,
        http_router::route_handler_t handler
    ) override;

    void
    handle_error(std::string_view reason);

private:
    std::unique_ptr<hfs::http_request> __req;
    std::unique_ptr<hfs::http_response> __res;

    int
    __server_static(int client_socket);
};

} // namespace hfs

#endif // __HFS_BLOCKING_HTTP_SERVER_H__
