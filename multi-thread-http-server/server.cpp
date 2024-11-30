#include "multi_thread_http_server.h"

int
main()
{
    hfs::http_server_base *server = new hfs::multi_thread_http_server();
    server->listen(7000);
    server->start();

    server->register_handler(
        "/", "GET",
        [](const hfs::http_request &req, hfs::http_response &res)
        {
            (void)req;
            (void)res;
            std::cout << "Hello, World!" << std::endl;
        }
    );

    delete server;
    return 0;
}
