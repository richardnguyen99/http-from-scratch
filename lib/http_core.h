#ifndef __HTTP_CORE_H__
#define __HTTP_CORE_H__ 1

// From cmake config to check whether a header is available
#include "http_config.h"

// Core C++ headers
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <ios>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

// Core POSIX headers
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Core network headers
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

// Third-party headers
#include <nlohmann/json.hpp> // For working with JSON structure
#include <pantor/inja.hpp>   // For working with Jinja2-like templates
#include <uriparser/Uri.h>   // For parsing URIs

// UUID headers
#ifdef HAVE_UUID_UUID_H
#include <uuid/uuid.h>
#endif

#ifdef HAVE_CSTDBOOL_H
#include <cstdbool>
#else
typedef enum
{
    false = 0,
    true  = 1,
} bool;
#endif

#ifdef HAVE_CSTDDEF_H
#include <cstddef>
#else
typedef long ptrdiff_t;
typedef unsigned long size_t;
typedef long ssize_t
#endif

namespace hfs
{

static constexpr const char *HTTP_SERVER_NAME          = "hfs";
static constexpr const char *HTTP_SERVER_VERSION       = "HTTP/1.1";
static constexpr std::size_t HTTP_SERVER__DEFAULT_PORT = 7000;
static constexpr std::size_t HTTP_BUFSZ                = 8192; // 8KB

static constexpr const char template_error[] = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>{{ status_code }} {{ status_text }}</title>
</head>
<body>
    <h1>{{ status_code }} {{ status_text }}</h1>
    <p>{{ message }}</p>
</body>
</html>
)";

typedef enum http_status_code
{
    /* Request fulfilled */
    HTTP_STATUS_OK         = 200,
    HTTP_STATUS_CREATED    = 201,
    HTTP_STATUS_ACCEPTED   = 202,
    HTTP_STATUS_NO_CONTENT = 204,

    /* Redirection */
    HTTP_STATUS_MOVED_PERMANENTLY = 301,
    HTTP_STATUS_FOUND             = 302,

    /* Client errors */
    HTTP_STATUS_BAD_REQUEST        = 400,
    HTTP_STATUS_UNAUTHORIZED       = 401,
    HTTP_STATUS_FORBIDDEN          = 403,
    HTTP_STATUS_NOT_FOUND          = 404,
    HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
    HTTP_STATUS_REQUEST_TIMEOUT    = 408,

    /* Server errors */
    HTTP_STATUS_INTERNAL_SERVER_ERROR      = 500,
    HTTP_STATUS_NOT_IMPLEMENTED            = 501,
    HTTP_STATUS_BAD_GATEWAY                = 502,
    HTTP_STATUS_SERVICE_UNAVAILABLE        = 503,
    HTTP_STATUS_GATEWAY_TIMEOUT            = 504,
    HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED = 505,
} http_status_code_t;

inline static const char *
http_status_str(http_status_code_t status)
{
    switch (status)
    {
    case HTTP_STATUS_OK:
        return "OK";
    case HTTP_STATUS_CREATED:
        return "Created";
    case HTTP_STATUS_ACCEPTED:
        return "Accepted";
    case HTTP_STATUS_NO_CONTENT:
        return "No Content";
    case HTTP_STATUS_MOVED_PERMANENTLY:
        return "Moved Permanently";
    case HTTP_STATUS_FOUND:
        return "Found";
    case HTTP_STATUS_BAD_REQUEST:
        return "Bad Request";
    case HTTP_STATUS_UNAUTHORIZED:
        return "Unauthorized";
    case HTTP_STATUS_FORBIDDEN:
        return "Forbidden";
    case HTTP_STATUS_NOT_FOUND:
        return "Not Found";
    case HTTP_STATUS_METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
    case HTTP_STATUS_REQUEST_TIMEOUT:
        return "Request Timeout";
    case HTTP_STATUS_INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
    case HTTP_STATUS_NOT_IMPLEMENTED:
        return "Not Implemented";
    case HTTP_STATUS_BAD_GATEWAY:
        return "Bad Gateway";
    case HTTP_STATUS_SERVICE_UNAVAILABLE:
        return "Service Unavailable";
    case HTTP_STATUS_GATEWAY_TIMEOUT:
        return "Gateway Timeout";
    case HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED:
        return "HTTP Version Not Supported";
    default:
        return "Unknown";
    }
}

inline static const char *
http_mime(const std::string &ext)
{
    std::cout << "ext: " << ext << std::endl;

    if (ext == "html")
        return "text/html";
    else if (ext == "css")
        return "text/css";
    else if (ext == "js")
        return "text/javascript";
    else if (ext == "json")
        return "application/json";
    else if (ext == "png")
        return "image/png";
    else if (ext == "jpg" || ext == "jpeg")
        return "image/jpeg";
    else if (ext == "gif")
        return "image/gif";
    else if (ext == "svg")
        return "image/svg+xml";
    else if (ext == "ico")
        return "image/x-icon";
    else if (ext == "pdf")
        return "application/pdf";
    else if (ext == "zip")
        return "application/zip";
    else if (ext == "tar")
        return "application/x-tar";
    else if (ext == "gz")
        return "application/gzip";
    else if (ext == "bz2")
        return "application/x-bzip2";
    else if (ext == "mp3")
        return "audio/mpeg";
    else if (ext == "wav")
        return "audio/wav";
    else if (ext == "mp4")
        return "video/mp4";
    else if (ext == "webm")
        return "video/webm";
    else if (ext == "ogg")
        return "video/ogg";
    else
        return "application/octet-stream";
}

inline static std::string
etag(time_t mt, std::size_t len)
{
    std::stringstream ss;
    ss << "\"" << std::hex << mt << "-" << len << "\"";

    return ss.str();
}

inline static std::string
format_date(time_t t)
{
    struct tm tstruct;
    char buf[80];
    tstruct = *gmtime(&t);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &tstruct);

    return buf;
}

inline static std::string
current_date()
{
    return format_date(time(0));
}

inline static void
handle_syscall_error(int status, const std::string &prefix)
{
    if (status == -1)
    {
        std::cerr << prefix << ": " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

} // namespace hfs

#endif // __HTTP_CORE_H__
