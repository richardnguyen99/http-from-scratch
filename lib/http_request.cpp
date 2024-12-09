#include <http_request.h>

namespace hfs
{
http_request::http_request()
    : __status(HTTP_STATUS_OK), __method(""), __version(""), __body(""),
      __headers(), __params(), __buf("")
{
    this->__uuid = http_uuid::generate(this);
}

http_request::http_request(const std::string &buf)
    : __status(HTTP_STATUS_OK), __method(""), __version(""), __body(""),
      __headers(), __params(), __buf(buf)
{
    this->__uuid = http_uuid::generate(this);
    this->__parse();
}

http_request::http_request(const char *buf, size_t len)
    : __status(HTTP_STATUS_OK), __method(""), __version(""), __body(""),
      __headers(), __params(), __buf(buf, len)
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
      __version(version), __body(body), __headers(headers), __params()
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

void
http_request::set_status(http_status_code_t status) noexcept
{
    this->__status = status;
}

std::string_view
http_request::method() const noexcept
{
    return this->__method;
}

std::string_view
http_request::path() const noexcept
{
    return this->__path.uri();
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

void
http_request::set_body(std::string_view body) noexcept
{
    this->__body = body;
}

void
http_request::set_body(const char *body, size_t len) noexcept
{
    this->__body = std::string(body, len);
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

const std::string &
http_request::param(const std::string &key) const
{
    auto it = this->__params.find(std::string(key));
    if (it == this->__params.end())
    {
        throw std::out_of_range("http_request::param: Invalid parameter");
    }

    return it->second;
}

void
http_request::add_param(
    const std::string &key, const std::string &value
) noexcept
{
    this->__params[key] = value;
}

const std::string &
http_request::uuid() const noexcept
{
    return this->__uuid;
}

const std::string &
http_request::data() const noexcept
{
    return this->__buf;
}

void
http_request::set_data(std::string_view data) noexcept
{
    this->__buf = data;
}

void
http_request::parse(std::string_view buf)
{
    this->__buf = buf;
    this->__parse();
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
        this->__status = HTTP_STATUS_BAD_REQUEST;
        throw std::runtime_error(hfs::format_function_error(
            __FILE__, __LINE__, "Invalid request line: " + line
        ));
    }

    std::string uri;

    request_line >> this->__method;
    request_line >> uri;
    request_line >> this->__version;

    if (this->__method.empty() || uri.empty() || this->__version.empty())
    {
        this->__status = HTTP_STATUS_BAD_REQUEST;
        throw std::runtime_error(hfs::format_function_error(
            __FILE__, __LINE__, "Invalid request line: " + line
        ));
    }

    if (!__check_method(this->__method))
    {
        this->__status = HTTP_STATUS_METHOD_NOT_ALLOWED;
        throw std::runtime_error(hfs::format_function_error(
            __FILE__, __LINE__, "Unsupported HTTP method: " + this->__method
        ));
    }

    if (this->__version != HTTP_SERVER_VERSION)
    {
        this->__status = HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED;
        throw std::runtime_error(hfs::format_function_error(
            __FILE__, __LINE__, "Unsupported HTTP version: " + this->__version
        ));
    }

    try
    {
        this->__path = hfs::http_uri(uri);
    }
    catch (const std::runtime_error &e)
    {
        this->__status = HTTP_STATUS_BAD_REQUEST;
        throw std::runtime_error(
            hfs::format_function_error(__FILE__, __LINE__, e.what())
        );
    }
}

/**
 * @brief HTTP RFC 2616 Section 5.3 - Request Header Fields

        uri_a->scheme.first*
 */
void
http_request::__parse_headers(const std::string &line)
{
    // Perform a search for the colon character
    auto pos = line.find(':');
    if (pos == std::string::npos)
    {
        throw std::runtime_error(hfs::format_function_error(
            __FILE__, __LINE__, "Invalid header: " + line
        ));
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
        throw std::runtime_error(
#if defined(DEBUG)
            "http_request::__parse: " +
#else
            "header parsing error: " +
#endif
            std::string("Empty request buffer")
        );
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
        this->__status = HTTP_STATUS_BAD_REQUEST;

        throw std::runtime_error(
#if defined(DEBUG)
            "http_request::__parse: " +
#else
            "header parsing error: " +
#endif
            std::string(e.what())
        );
    }

    std::size_t hdr_field_start = line.size() + 2;
    std::size_t hdr_field_end   = hdr_field_start;

    // Parse the headers. `\r\n` indicates the end of the header section.
    while (std::getline(stream, line) && !line.empty() && line != "\r")
    {
        try
        {
            this->__parse_headers(line);
            hdr_field_end += line.size() + 2;
        }
        catch (const std::runtime_error &e)
        {
            this->__status = HTTP_STATUS_BAD_REQUEST;

            throw std::runtime_error(
#if defined(DEBUG)
                "http_request::__parse: " +
#else
                "header parsing error: " +
#endif
                std::string(e.what())
            );
        }
    }

    if (hdr_field_end - hdr_field_start >= hfs::HTTP_HDRSZ)
    {
        this->__status = HTTP_STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE;
        throw std::runtime_error(
            "http_request::__parse: Request header fields too large"
        );
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
