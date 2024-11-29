#include <http_request.h>

namespace hfs
{
http_request::http_request()
    : __status(HTTP_STATUS_OK), __method(""), __path(""), __version(""),
      __body(""), __headers(), __buf("")
{
    this->__uuid = http_uuid::generate(this);
}

http_request::http_request(const std::string &buf)
    : __status(HTTP_STATUS_OK), __method(""), __path(""), __version(""),
      __body(""), __headers(), __buf(buf)
{
    this->__uuid = http_uuid::generate(this);
    this->__parse();
}

http_request::http_request(const char *buf, size_t len)
    : __status(HTTP_STATUS_OK), __method(""), __path(""), __version(""),
      __body(""), __headers(), __buf(buf, len)
{
    this->__uuid = http_uuid::generate(this);
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
    this->__uuid = http_uuid::generate(this);
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
void
http_request::__parse_request_line(const std::string &line)
{
    std::istringstream request_line(line);

    if (!request_line.good())
    {
        throw std::runtime_error("Invalid request line: " + line);
    }

    request_line >> this->__method;
    request_line >> this->__path;
    request_line >> this->__version;

    if (this->__method.empty() || this->__path.empty() ||
        this->__version.empty())
    {
        throw std::runtime_error("Invalid request line: " + this->__method);
    }

    if (!__check_method(this->__method))
    {
        this->__status = HTTP_STATUS_NOT_IMPLEMENTED;
        throw std::runtime_error(
            "Unsupported request this->__method: " + this->__method
        );
    }

    if (this->__version != HTTP_SERVER_VERSION)
    {
        this->__status = HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED;
        throw std::runtime_error(
            "Unsupported HTTP this->__version: " + this->__version
        );
    }
}

/**
 * @brief HTTP RFC 2616 Section 5.3 - Request Header Fields
 *
 */
void
http_request::__parse_headers(const std::string &line)
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

    this->__headers[name] = value;
}

void
http_request::__parse()
{
    if (this->__buf.empty())
    {
        this->__status = HTTP_STATUS_BAD_REQUEST;
        throw std::runtime_error("http_request::__parse: Invalid request");
    }

    std::string line;
    std::istringstream stream(this->__buf);

    // Parse the request line
    std::getline(stream, line);

    try
    {
        this->__parse_request_line(line);
    }
    catch (const std::runtime_error &e)
    {
        // In case of an unset error, set the status to `400 Bad Request`.
        if (this->__status == HTTP_STATUS_OK)
        {
            this->__status = HTTP_STATUS_BAD_REQUEST;
        }

        throw std::runtime_error(
            "http_request::__parse: " + std::string(e.what())
        );
    }

    // Parse the headers. `\r\n` indicates the end of the header section.
    while (std::getline(stream, line) && !line.empty() && line != "\r")
    {
        try
        {
            this->__parse_headers(line);
        }
        catch (const std::runtime_error &e)
        {
            // In case of an unset error, set the status to `400 Bad Request`.
            if (this->__status == HTTP_STATUS_OK)
            {
                this->__status = HTTP_STATUS_BAD_REQUEST;
            }

            throw std::runtime_error(
                "http_request::__parse: " + std::string(e.what())
            );
        }
    }
}

std::ostream &
operator<<(std::ostream &os, const http_request &req)
{
    os << "Req(id = " << req.__uuid << ")\n"
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
