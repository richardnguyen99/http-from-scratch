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
            std::ifstream f("../public/pages/index.html");
            std::string body(
                (std::istreambuf_iterator<char>(f)),
                std::istreambuf_iterator<char>()
            );

            res.status(hfs::HTTP_STATUS_OK)
                .header("Content-Type", "text/html")
                .header("Content-Length", std::to_string(body.length()))
                .header("X-Request-ID", req.uuid())
                .header("Server", "HFS")
                .header("Connection", "close")
                .body(body);

            f.close();
        }
    );

    server->start();

    delete server;
    return 0;
}
