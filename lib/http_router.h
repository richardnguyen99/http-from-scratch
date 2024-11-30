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

    static void
    not_implemented(const hfs::http_request &req, hfs::http_response &res)
    {
        (void)req;
        (void)res;
        throw std::runtime_error("Not implemented");
    };

public:
    http_router();
    ~http_router();

    std::string base_name;
    std::unordered_map<std::string, hfs::http_router> routes;
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
