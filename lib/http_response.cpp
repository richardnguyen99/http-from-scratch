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
inja::Environment http_response::env = inja::Environment();

http_response::http_response()
    : __status(HTTP_STATUS_OK), __headers(), __body(""), __page_dir("")
{
}

http_response::http_response(const std::string &page_dir)
    : __status(HTTP_STATUS_OK), __headers(), __body(""), __page_dir(page_dir)
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

http_status_code_t
http_response::status() const
{
    return this->__status;
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

http_response &
http_response::render(const std::string &endpoint, inja::json data, int flags)
{
    (void)data;

    int fd;
    struct stat st;
    char *buffer;

    std::string template_path = this->__page_dir + "/" + endpoint + ".html";

    if ((fd = open(template_path.c_str(), O_RDONLY)) == -1)
    {
        if (errno == ENOENT)
        {
            this->status(HTTP_STATUS_NOT_FOUND);
            this->body("404 Not Found");
            return *this;
        }
        else
        {
            this->status(HTTP_STATUS_INTERNAL_SERVER_ERROR);
            this->body("500 Internal Server Error");
            return *this;
        }
    }

    if (fstat(fd, &st) == -1)
    {
        this->status(HTTP_STATUS_INTERNAL_SERVER_ERROR);
        this->body("500 Internal Server Error");
        close(fd);
        return *this;
    }

    buffer = (char *)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (buffer == MAP_FAILED)
    {
        this->status(HTTP_STATUS_INTERNAL_SERVER_ERROR);
        this->body("500 Internal Server Error");
        close(fd);
        return *this;
    }

    if (close(fd) == -1)
    {
        this->status(HTTP_STATUS_INTERNAL_SERVER_ERROR);
        this->body("500 Internal Server Error");
        munmap(buffer, st.st_size);
        return *this;
    }

    if (flags & GET_REQUEST)
    {
        std::string temp(buffer, st.st_size);
        std::string body = hfs::http_response::env.render(temp, data);
        this->body(body).header("Content-Type", "text/html; charset=utf-8");
    }

    if (flags & ETAG)
        this->header("ETag", "W/" + hfs::etag(st.st_mtime, st.st_size));

    if (flags & LAST_MODIFIED)
        this->header("Last-Modified", hfs::format_date(st.st_mtime));

    this->header("Cache-Control", "public, max-age=0, must-revalidate");

    if (munmap(buffer, st.st_size) == -1)
    {
        this->status(HTTP_STATUS_INTERNAL_SERVER_ERROR);
        this->body("500 Internal Server Error");
        return *this;
    }

    return *this;
}

http_response &
http_response::render(const std::string &endpoint)
{
    inja::json data;
    return this->render(endpoint, data);
}
} // namespace hfs
