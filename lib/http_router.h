#ifndef __HTTP_ROUTE_H__
#define __HTTP_ROUTE_H__ 1

#include <http_core.h>
#include <http_request.h>
#include <http_response.h>
#include <http_uri.h>

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
    not_implemented(const hfs::http_request &req, hfs::http_response &res);

    static void
    default_error_handler(
        hfs::http_status_code_t status, const std::string &reason,
        const hfs::http_request &req, hfs::http_response &res
    );

    static std::pair<hfs::http_router *, hfs::http_router::route_handler_t>
    get_route_handler(hfs::http_router *router, hfs::http_request *req);

public:
    http_router();
    ~http_router();

    std::string base_name;
    bool is_param_router;
    std::unordered_map<std::string, std::unique_ptr<hfs::http_router>> routes;
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

protected:
    http_router(bool is_param_router);
};

class http_param_router : public http_router
{
public:
    http_param_router();
    ~http_param_router();

    std::string param_name;
};

} // namespace hfs
#endif // __HTTP_ROUTE_H__
