#include "multi_process_http_server.h"

namespace hfs
{
multi_process_http_server::multi_process_http_server()
{
    std::cout << "multi_process_http_server::multi_process_http_server()"
              << std::endl;
}

multi_process_http_server::~multi_process_http_server()
{
    std::cout << "multi_process_http_server::~multi_process_http_server()"
              << std::endl;
}

void
multi_process_http_server::start()
{
    std::cout << "multi_process_http_server::start()" << std::endl;
}

void
multi_process_http_server::listen(int port, int backlog, int optval)
{
    std::cout << "multi_process_http_server::listen(" << port << ")"
              << std::endl;

    this->__port        = port;
    this->__backlog     = backlog;
    this->__socket_flag = optval;
}

void
multi_process_http_server::register_handler(
    const std::string &path, const std::string &method,
    hfs::http_router::route_handler_t handler
)
{
    (void)handler;
    std::cout << "multi_process_http_server::register_handler(" << path << ", "
              << method << ")" << std::endl;
}

void
multi_process_http_server::register_static_handler(const std::string &path)
{
    std::cout << "multi_process_http_server::register_static(" << path << ")"
              << std::endl;
}

void
multi_process_http_server::register_error_handler(
    hfs::http_status_code_t status_code, const std::string &path,
    hfs::http_router::route_handler_t handler
)
{
    (void)status_code;
    (void)path;
    (void)handler;
}
} // namespace hfs
