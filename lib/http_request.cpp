#include <http_request.h>

namespace hfs
{
http_request::http_request()
    : __status(HTTP_STATUS_OK), __method(""), __path(""), __version(""),
      __body(""), __headers(), __buf("")
{
}

http_request::http_request(const std::string &buf)
    : __status(HTTP_STATUS_OK), __method(""), __path(""), __version(""),
      __body(""), __headers(), __buf(buf)
{
    this->__parse();
}

http_request::http_request(const char *buf, size_t len)
    : __status(HTTP_STATUS_OK), __method(""), __path(""), __version(""),
      __body(""), __headers(), __buf(buf, len)
{
    this->__parse();
}

http_request::http_request(
    const std::string &method, const std::string &path,
    const std::string &version, const std::string &body,
    const std::unordered_map<std::string, std::string> &headers
)
    : __status(HTTP_STATUS_OK), __method(method), __path(path),
      __version(version), __body(body), __headers(headers)
{
}

http_request::~http_request()
{
}

http_status_code_t
http_request::status() const noexcept
{
    return this->__status;
}

const std::string &
http_request::method() const noexcept
{
    return this->__method;
}

const std::string &
http_request::path() const noexcept
{
    return this->__path;
}

const std::string &
http_request::version() const noexcept
{
    return this->__version;
}

const std::string &
http_request::body() const noexcept
{
    return this->__body;
}

const std::string &
http_request::header(const std::string &key) const
{
    auto it = this->__headers.find(key);
    if (it == this->__headers.end())
    {
        throw std::out_of_range("http_request::header: Invalid header");
    }

    return it->second;
}

static bool
__check_method(const std::string &method)
{
    static const std::vector<std::string> methods = {
        "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"};

    return std::find(methods.begin(), methods.end(), method) != methods.end();
}

/**
 * @brief HTTP RFC 2616 Section 5.1 - Request-Line
 *
 * Request-Line = Method SP Request-URI SP HTTP-Version CRLF
 *
 * @example
 * GET /index.html HTTP/1.1
 * POST /login HTTP/1.1
 */
static void
__parse_request_line(
    const std::string &line, std::string &method, std::string &path,
    std::string &version
)
{
    std::istringstream request_line(line);

    if (!request_line.good())
    {
        throw std::runtime_error("Invalid request line: " + line);
    }

    request_line >> method;
    request_line >> path;
    request_line >> version;

    if (method.empty() || path.empty() || version.empty())
    {
        throw std::runtime_error("Invalid request line: " + method);
    }

    if (!__check_method(method))
    {
        throw std::runtime_error("Unsupported request method: " + method);
    }

    if (version != HTTP_SERVER_VERSION)
    {
        throw std::runtime_error("Unsupported HTTP version: " + version);
    }
}

/**
 * @brief HTTP RFC 2616 Section 5.3 - Request Header Fields
 *
 */
static void
__parse_headers(
    const std::string &line,
    std::unordered_map<std::string, std::string> &headers
)
{
    // Perform a search for the colon character
    auto pos = line.find(':');
    if (pos == std::string::npos)
    {
        throw std::runtime_error("Invalid header: " + line);
    }

    // Extract the header name and value
    std::string name  = line.substr(0, pos);
    std::string value = line.substr(pos + 1);

    // Remove leading and trailing whitespaces
    name.erase(std::remove_if(name.begin(), name.end(), ::isspace), name.end());

    value.erase(
        std::remove_if(value.begin(), value.end(), ::isspace), value.end()
    );

    headers[name] = value;
}

void
http_request::__parse()
{
    if (this->__buf.empty())
    {
        throw std::runtime_error("http_request::__parse: Invalid request");
    }

    std::string line;
    std::istringstream stream(this->__buf);

    // Parse the request line
    std::getline(stream, line);

    try
    {
        __parse_request_line(
            line, this->__method, this->__path, this->__version
        );
    }
    catch (const std::runtime_error &e)
    {
        throw std::runtime_error(
            "http_request::__parse: " + std::string(e.what())
        );
    }

    // Parse the headers. `\r\n` indicates the end of the header section.
    while (std::getline(stream, line) && !line.empty() && line != "\r")
    {
        try
        {
            __parse_headers(line, this->__headers);
        }
        catch (const std::runtime_error &e)
        {
            throw std::runtime_error(
                "http_request::__parse: " + std::string(e.what())
            );
        }
    }
}

std::ostream &
operator<<(std::ostream &os, const http_request &req)
{
    os << "Req\n"
       << "├── method: " << req.method() << "\n"
       << "├── url: " << req.path() << "\n"
       << "├── version: " << req.version() << "\n"
       << "└── headers:\n";

    size_t i = 0;
    for (const auto &[name, value] : req.__headers)
    {
        if (i++ == req.__headers.size() - 1)
        {
            os << "    └── " << name << ": " << value << "\n";
            continue;
        }

        os << "    ├── " << name << ": " << value << "\n";
    }

    return os;
}

} // namespace hfs
