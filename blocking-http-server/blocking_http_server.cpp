#include "blocking_http_server.h"

namespace hfs
{
blocking_http_server::blocking_http_server()
{
    std::cout << "blocking_http_server::blocking_http_server()" << std::endl;
}

blocking_http_server::~blocking_http_server()
{
    std::cout << "blocking_http_server::~blocking_http_server()" << std::endl;
}

void
blocking_http_server::start()
{
    std::cout << "blocking_http_server::start()" << std::endl;
}

void
blocking_http_server::listen(int port)
{
    std::cout << "blocking_http_server::listen(" << port << ")" << std::endl;
}

void
blocking_http_server::register_handler(
    const std::string &path, const std::string &method,
    std::function<void()> handler
)
{
    std::cout << "blocking_http_server::register_handler(" << path << ", "
              << method << ")" << std::endl;

    handler();
}
} // namespace hfs
