#include <http_router.h>

namespace hfs
{

http_router::http_router() : base_name("/"), is_param_router(false)
{
}

http_router::http_router(bool is_param_router)
    : base_name("/"), is_param_router(is_param_router)
{
}

http_router::~http_router()
{
}

http_param_router::http_param_router() : http_router(true)
{
}

http_param_router::~http_param_router()
{
}

void
http_router::not_implemented(
    const hfs::http_request &req, hfs::http_response &res
)
{
    (void)req;
    res.status(hfs::HTTP_STATUS_NOT_IMPLEMENTED);

    throw std::runtime_error(
        "The request is valid but the handler '" + std::string(req.method()) +
        "' at resource '" + std::string(req.path()) + "' is not implemented"
    );
};

void
http_router::default_error_handler(
    hfs::http_status_code_t status, std::string_view reason,
    const hfs::http_request &req, hfs::http_response &res
)
{
    (void)req;

    std::string body = hfs::http_response::env.render(
        hfs::template_error,
        {
            {"status_code", std::to_string(status)      },
            {"status_text", hfs::http_status_str(status)},
            {"message",     reason                      },
    }
    );

    res.status(status)
        .header("Content-Type", "text/html; charset=utf-8")
        .header("Connection", "close")
        .header("Date", hfs::current_date())
        .header("Cache-Control", "no-cache, no-store, must-revalidate")
        .header("Pragma", "no-cache")
        .header("Expires", "-1")
        .header(
            "Server", std::string(hfs::HTTP_SERVER_NAME) + "/" +
                          std::string(hfs::HTTP_SERVER_VERSION)
        )
        .header("ETag", "W/" + hfs::etag(time(0), body.size()))
        .body(body);
}

std::pair<hfs::http_router *, hfs::http_router::route_handler_t>
http_router::get_route_handler(
    hfs::http_router *root_router, hfs::http_request *req
)
{
    std::vector<std::string> path_parts =
        hfs::http_uri::split_path(std::string(req->path()));

    hfs::http_router::route_handler_t handler = nullptr;
    hfs::http_router *router                  = root_router;
    std::unordered_map<std::string, std::string> params;

    // Find the handler for the path
    for (const auto &part : path_parts)
    {
        if (part == "/")
        {
            router = root_router;
        }
        else if (router->routes.find(part) != router->routes.end())
        {
            router = router->routes[part].get();
        }
        // Check if the router has a parameter router
        else if (router->routes.find("*") != router->routes.end())
        {
            router = router->routes["*"].get();
            req->add_param(((http_param_router *)router)->param_name, part);
        }
        // Find in the static files
        else
        {
            return std::make_pair(nullptr, nullptr);
        }
    }

    handler = router->handlers[std::string(req->method())];

    return std::make_pair(router, handler);
}
} // namespace hfs
