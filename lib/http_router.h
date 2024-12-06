#ifndef __HTTP_ROUTE_H__
#define __HTTP_ROUTE_H__ 1

#include <http_core.h>
#include <http_request.h>
#include <http_response.h>

namespace hfs
{
class http_router
{
public:
    using route_handler_t = std::function<
        void(const hfs::http_request &req, hfs::http_response &res)>;

    using error_handler_t = std::function<void(
        hfs::http_status_code_t status, const std::string &reason,
        const hfs::http_request &req, hfs::http_response &res
    )>;

    static void
    not_implemented(const hfs::http_request &req, hfs::http_response &res)
    {
        (void)req;
        (void)res;
        throw std::runtime_error("Not implemented");
    };

    static void
    default_error_handler(
        hfs::http_status_code_t status, const std::string &reason,
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

public:
    http_router();
    ~http_router();

    std::string base_name;
    std::unordered_map<std::string, hfs::http_router> routes;
    std::unordered_map<hfs::http_status_code_t, error_handler_t> error_handlers;
    std::unordered_map<std::string, route_handler_t> handlers = {
        {"GET",     not_implemented},
        {"POST",    not_implemented},
        {"PUT",     not_implemented},
        {"DELETE",  not_implemented},
        {"PATCH",   not_implemented},
        {"OPTIONS", not_implemented},
        {"HEAD",    not_implemented},
        {"TRACE",   not_implemented},
        {"CONNECT", not_implemented},
    };
};
} // namespace hfs
#endif // __HTTP_ROUTE_H__
