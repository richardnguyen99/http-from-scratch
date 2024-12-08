#include "http_uri.h"

namespace hfs
{
http_uri::http_uri()
{
}

http_uri::http_uri(std::string_view uri) : __uri(uri)
{
    UriUriA *uri_a = this->parse(uri);
    if (uri_a == nullptr)
    {
        throw std::runtime_error("Invalid URI: " + std::string(uri));
    }

    this->__scheme =
        uri_a->scheme.first
            ? std::string(uri_a->scheme.first, uri_a->scheme.afterLast)
            : "";

    this->__host =
        uri_a->hostText.first
            ? std::string(uri_a->hostText.first, uri_a->hostText.afterLast)
            : "";

    this->__port =
        uri_a->portText.first
            ? std::string(uri_a->portText.first, uri_a->portText.afterLast)
            : "";

    UriPathSegmentA *segment;
    for (segment = uri_a->pathHead; segment != nullptr; segment = segment->next)
    {
        this->__path.push_back(
            std::string(segment->text.first, segment->text.afterLast)
        );
    }

    std::string query =
        uri_a->query.first
            ? std::string(uri_a->query.first, uri_a->query.afterLast)
            : "";

    // Parser the query string
    std::istringstream query_stream(query);
    for (std::string part; std::getline(query_stream, part, '&');)
    {
        std::istringstream part_stream(part);
        std::string key, value;

        if (std::getline(part_stream, key, '='))
        {
            if (std::getline(part_stream, value))
            {
                this->__query[key] = value;
            }
        }
    }

    this->__fragment =
        uri_a->fragment.first
            ? std::string(uri_a->fragment.first, uri_a->fragment.afterLast)
            : "";

    uriFreeUriMembersA(uri_a);
    delete uri_a;
}

http_uri::~http_uri()
{
}

std::string_view
http_uri::uri() const noexcept
{
    return this->__uri;
}

std::string_view
http_uri::path() const noexcept
{
    std::stringstream path_stream;

    for (const auto &part : this->__path)
    {
        path_stream << "/" << part;
    }

    return path_stream.str();
}

std::string_view
http_uri::query(std::string_view key) const noexcept
{
    auto it = this->__query.find(std::string(key));

    if (it == this->__query.end())
    {
        return "";
    }

    return it->second;
}

std::string_view
http_uri::fragment() const noexcept
{
    return this->__fragment;
}

std::string_view
http_uri::scheme() const noexcept
{
    return this->__scheme;
}

std::string_view
http_uri::host() const noexcept
{
    return this->__host;
}

std::string_view
http_uri::port() const noexcept
{
    return this->__port;
}

UriUriA *
http_uri::parse(std::string_view uri)
{
    UriUriA *uri_a      = new UriUriA();
    const char *uri_str = uri.data();
    const char *error_pos;

    if (uriParseSingleUriA(uri_a, uri_str, &error_pos) != URI_SUCCESS)
    {
        uriFreeUriMembersA(uri_a);
        delete uri_a;
        return nullptr;
    }

    return uri_a;
}

std::vector<std::string>
http_uri::split_path(std::string_view path)
{
    UriUriA *uri_a = http_uri::parse(path);
    if (uri_a == nullptr)
    {
        throw std::runtime_error("Invalid path: " + std::string(path));
    }

    std::vector<std::string> segments;
    UriPathSegmentA *segment;

    for (segment = uri_a->pathHead; segment != nullptr; segment = segment->next)
    {
        segments.push_back(
            std::string(segment->text.first, segment->text.afterLast)
        );
    }

    uriFreeUriMembersA(uri_a);
    delete uri_a;

    return segments;
}
} // namespace hfs
