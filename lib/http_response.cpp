#include <http_response.h>

static std::string
__current_date()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *gmtime(&now);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &tstruct);

    return buf;
}

namespace hfs
{
http_response::http_response()
    : __status(HTTP_STATUS_OK), __headers(), __body("")
{
}

http_response::~http_response()
{
}

std::string
http_response::operator()() const
{
    std::stringstream response;
    response << "HTTP/1.1 " << this->__status << " "
             << http_status_str(this->__status) << "\r\n";

    for (const auto &[key, value] : this->__headers)
    {
        response << key << ": " << value << "\r\n";
    }

    response << "\r\n" << this->__body;

    return response.str();
}

http_response &
http_response::status(http_status_code_t status)
{
    this->__status = status;
    return *this;
}

http_response &
http_response::header(const std::string &key, const std::string &value)
{
    this->__headers[key] = value;
    return *this;
}

http_response &
http_response::body(const std::string &body)
{
    // Headers prepared for generic text content
    this->__headers["Content-Length"] = std::to_string(body.length());
    this->__headers["Date"]           = __current_date();

    this->__body = body;
    return *this;
}
} // namespace hfs
