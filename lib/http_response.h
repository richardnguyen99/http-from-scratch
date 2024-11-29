#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__ 1

#include <http_core.h>

namespace hfs
{
class http_response
{
public:
    http_response();
    ~http_response();

    std::string
    operator()() const;

    http_response &
    status(http_status_code_t status);

    http_response &
    header(const std::string &key, const std::string &value);

    http_response &
    body(const std::string &body);

private:
    http_status_code_t __status;
    std::unordered_map<std::string, std::string> __headers;
    std::string __body;
};
} // namespace hfs

#endif // __HTTP_RESPONSE_H__
