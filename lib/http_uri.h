#ifndef __HTTP_URI_H__
#define __HTTP_URI_H__ 1

#include <http_core.h>

namespace hfs
{
class http_uri
{
public:
    explicit http_uri(const std::string &uri);
    ~http_uri();

    static UriUriA *
    parse(const std::string &uri);

private:
    std::string uri;
    std::string path;
    std::string query;
    std::string fragment;
};
} // namespace hfs

#endif // __HTTP_URI_H__
