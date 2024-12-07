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
} // namespace hfs
