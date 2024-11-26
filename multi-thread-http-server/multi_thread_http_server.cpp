#include "multi_thread_http_server.h"

namespace hfs
{
multi_thread_http_server::multi_thread_http_server()
{
    std::cout << "multi_thread_http_server::multi_thread_http_server()"
              << std::endl;
}

multi_thread_http_server::~multi_thread_http_server()
{
    std::cout << "multi_thread_http_server::~multi_thread_http_server()"
              << std::endl;
}

void
multi_thread_http_server::start()
{
    std::cout << "multi_thread_http_server::start()" << std::endl;
}

void
multi_thread_http_server::listen(int port, int backlog, int optval)
{
    std::cout << "multi_thread_http_server::listen(" << port << ")"
              << std::endl;

    this->__port        = port;
    this->__backlog     = backlog;
    this->__socket_flag = optval;
}

void
multi_thread_http_server::register_handler(
    const std::string &path, const std::string &method,
    std::function<void()> handler
)
{
    std::cout << "multi_thread_http_server::register_handler(" << path << ", "
              << method << ")" << std::endl;

    handler();
}
} // namespace hfs
