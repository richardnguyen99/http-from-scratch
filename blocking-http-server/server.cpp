#include <http_core.h>
#include <http_server.h>

#include "blocking_http_server.h"

int
main()
{
    hfs::http_server_base *server = new hfs::blocking_http_server();
    server->listen(7000);

    server->register_handler(
        "/", "GET",
        [](const hfs::http_request &req, hfs::http_response &res)
        {
            std::cout << "GET_handler" << std::endl;
            res.status(hfs::HTTP_STATUS_OK)
                .header("Content-Type", "text/html")
                .header("Server", "HFS")
                .header("Connection", "close")
                .header("X-Request-ID", req.uuid())
                .body("<html>"
                      "<head>"
                      "<title>HTTP Server</title>"
                      "</head>"
                      "<body>"
                      "<h1>Hello, World!</h1>"
                      "</body>"
                      "</html>");
        }
    );

    server->start();

    delete server;
    return 0;
}
