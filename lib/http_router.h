#ifndef __HTTP_ROUTE_H__
#define __HTTP_ROUTE_H__ 1

#include <http_core.h>
#include <http_request.h>
#include <http_response.h>

namespace hfs
{
class http_router
{
    using route_handler_t = std::function<
        void(const hfs::http_request &req, hfs::http_response &res)>;

public:
    http_router();
    ~http_router();

    route_handler_t GET;
    route_handler_t POST;
    route_handler_t PUT;
    route_handler_t DELETE;
    route_handler_t PATCH;
    route_handler_t OPTIONS;
    route_handler_t HEAD;
    route_handler_t TRACE;
    route_handler_t CONNECT;
    std::string base_name;
    std::unordered_map<std::string, hfs::http_router> routes;
};

} // namespace hfs
#endif // __HTTP_ROUTE_H__
