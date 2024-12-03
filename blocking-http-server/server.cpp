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
            (void)req;
            res.render("index");
        }
    );

    server->start();

    delete server;
    return 0;
}
