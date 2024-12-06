#include "http_uri.h"

namespace hfs
{
http_uri::http_uri(const std::string &uri) : uri(uri)
{
}

http_uri::~http_uri()
{
}

UriUriA *
http_uri::parse(const std::string &uri)
{
    UriUriA *uri_a      = new UriUriA();
    const char *uri_str = uri.c_str();
    const char *error_pos;

    if (uriParseSingleUriA(uri_a, uri_str, &error_pos) != URI_SUCCESS)
    {
        uriFreeUriMembersA(uri_a);
        delete uri_a;
        return nullptr;
    }

    return uri_a;
}
} // namespace hfs
