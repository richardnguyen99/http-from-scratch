#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__ 1

#include <http_core.h>

namespace hfs
{
class http_request
{
public:
    http_request();

    explicit http_request(const std::string &buf);

    explicit http_request(const char *buf, size_t len);

    http_request(
        const std::string &method, const std::string &path,
        const std::string &version, const std::string &body,
        const std::unordered_map<std::string, std::string> &headers
    );

    ~http_request();

private:
    http_status_code_t __status;
    std::string __method;
    std::string __path;
    std::string __version;
    std::string __body;
    std::unordered_map<std::string, std::string> __headers;

    void
    __parse(const char *buf, size_t len);
};
} // namespace hfs

#endif // __HTTP_REQUEST_H__
