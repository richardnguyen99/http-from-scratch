#include <http_core.h>
#include <http_server.h>

#include "blocking_http_server.h"

int
main()
{
    hfs::http_server_base *server = new hfs::blocking_http_server();

    server->register_handler(
        "/", "GET",
        [](const hfs::http_request &req, hfs::http_response &res)
        {
            (void)req;
            res.render("index");
        }
    );

    server->register_handler(
        "/login", "GET",
        [](const hfs::http_request &req, hfs::http_response &res)
        {
            (void)req;
            res.render("login");
        }
    );

    server->register_handler(
        "/login", "POST",
        [](const hfs::http_request &req, hfs::http_response &res)
        {
            (void)req;

            inja::json data;
            data["title"]   = "Login";
            data["message"] = "Login successful!";

            res.render("login", data);
        }
    );

    server->register_handler(
        "/register", "GET",
        [](const hfs::http_request &req, hfs::http_response &res)
        {
            (void)req;
            res.render("register");
        }
    );

    server->register_handler(
        "/register", "POST",
        [](const hfs::http_request &req, hfs::http_response &res)
        {
            (void)req;

            inja::json data;
            data["title"]   = "Register";
            data["message"] = "Registration successful!";

            res.render("register", data);
        }
    );

    server->register_handler(
        "/about", "GET",
        [](const hfs::http_request &req, hfs::http_response &res)
        {
            (void)req;

            inja::json data;
            data["title"] = "About";

            res.render("about", data);
        }
    );

    server->register_handler(
        "/blogs/:slug", "GET",
        [](const hfs::http_request &req, hfs::http_response &res)
        {
            std::string slug = req.param("slug");

            inja::json data;
            data["heading"] = slug;
            data["title"]   = slug + " | Blog";

            res.render("blog", data);
        }
    );

    server->listen(7000);
    server->start();

    delete server;
    return 0;
}
