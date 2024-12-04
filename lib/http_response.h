#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__ 1

#include <http_core.h>

namespace hfs
{

class http_response
{
public:
    static inja::Environment env;
    static const int HEAD_REQUEST  = 0b0000000;
    static const int GET_REQUEST   = 0b0000001;
    static const int ETAG          = 0b0000010;
    static const int LAST_MODIFIED = 0b0000100;

    http_response();
    http_response(const std::string &page_dir);
    ~http_response();

    std::string
    operator()() const;

    http_response &
    status(http_status_code_t status);

    http_response &
    header(const std::string &key, const std::string &value);

    http_response &
    body(const std::string &body);

    http_response &
    render(
        const std::string &endpoint, inja::json data,
        int flags = GET_REQUEST | ETAG | LAST_MODIFIED
    );

    http_response &
    render(const std::string &endpoint);

private:
    http_status_code_t __status;
    std::unordered_map<std::string, std::string> __headers;
    std::string __body;
    std::string __page_dir;
};
} // namespace hfs

#endif // __HTTP_RESPONSE_H__
