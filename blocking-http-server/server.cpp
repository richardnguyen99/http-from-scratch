#include <http_core.h>
#include <http_server.h>

#include "blocking_http_server.h"

int
main()
{
    hfs::http_server_base *server = new hfs::blocking_http_server();
    server->listen(7000);
    server->start();
    server->register_handler(
        "/", "GET", []() { std::cout << "Hello, World!" << std::endl; }
    );

    delete server;
    return 0;
}
