#ifndef __HTTP_CORE_H__
#define __HTTP_CORE_H__ 1

// From cmake config to check whether a header is available
#include <http_config.h>

// Core C++ headers
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// Core POSIX headers
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Core network headers
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

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

#define HTTP_SERVER_NAME          "http-from-scratch server"
#define HTTP_SERVER_VERSION       "HTTP/1.1"
#define HTTP_SERVER__DEFAULT_PORT 7000

typedef enum http_status_code
{
    HTTP_STATUS_OK         = 200,
    HTTP_STATUS_CREATED    = 201,
    HTTP_STATUS_ACCEPTED   = 202,
    HTTP_STATUS_NO_CONTENT = 204,

    HTTP_STATUS_MOVED_PERMANENTLY = 301,
    HTTP_STATUS_FOUND             = 302,

    HTTP_STATUS_BAD_REQUEST        = 400,
    HTTP_STATUS_UNAUTHORIZED       = 401,
    HTTP_STATUS_FORBIDDEN          = 403,
    HTTP_STATUS_NOT_FOUND          = 404,
    HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
    HTTP_STATUS_REQUEST_TIMEOUT    = 408,

    HTTP_STATUS_INTERNAL_SERVER_ERROR = 500,
    HTTP_STATUS_NOT_IMPLEMENTED       = 501,
    HTTP_STATUS_BAD_GATEWAY           = 502,
    HTTP_STATUS_SERVICE_UNAVAILABLE   = 503,
} http_status_code_t;

#endif // __HTTP_CORE_H__
