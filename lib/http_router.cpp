#include <http_router.h>

namespace hfs
{
auto not_implemented = [](const hfs::http_request &req, hfs::http_response &res)
{
    (void)req;
    (void)res;
    throw std::runtime_error("Not implemented");
};

http_router::http_router()
{
    this->GET       = not_implemented;
    this->POST      = not_implemented;
    this->PUT       = not_implemented;
    this->DELETE    = not_implemented;
    this->PATCH     = not_implemented;
    this->OPTIONS   = not_implemented;
    this->HEAD      = not_implemented;
    this->TRACE     = not_implemented;
    this->CONNECT   = not_implemented;
    this->base_name = "/";
}

http_router::~http_router()
{
}
} // namespace hfs
