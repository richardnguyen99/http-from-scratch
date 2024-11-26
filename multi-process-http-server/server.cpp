#include "multi_process_http_server.h"

int
main()
{
    hfs::http_server_base *server = new hfs::multi_process_http_server();
    server->listen(7000);
    server->start();
    server->register_handler(
        "/", "GET", []() { std::cout << "Hello, World!" << std::endl; }
    );

    delete server;
    return 0;
}
