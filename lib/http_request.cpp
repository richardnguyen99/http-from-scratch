#include <http_request.h>

namespace hfs
{
http_request::http_request()
    : __method(""), __path(""), __version(""), __body(""), __headers()
{
}

http_request::http_request(const std::string &buf)
    : __method(""), __path(""), __version(""), __body(""), __headers()
{
    this->__parse(buf.c_str(), buf.length());
}

http_request::http_request(const char *buf, size_t len)
    : __method(""), __path(""), __version(""), __body(""), __headers()
{
    this->__parse(buf, len);
}

http_request::http_request(
    const std::string &method, const std::string &path,
    const std::string &version, const std::string &body,
    const std::unordered_map<std::string, std::string> &headers
)
    : __method(method), __path(path), __version(version), __body(body),
      __headers(headers)
{
}

http_request::~http_request()
{
}

void
http_request::__parse(const char *buf, size_t len)
{
    std::cout << "http_request::__parse(" << buf << ", " << len << ")"
              << std::endl;
}

} // namespace hfs
