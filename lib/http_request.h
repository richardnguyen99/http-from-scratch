#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__ 1

#include <http_core.h>
#include <http_uuid.h>

namespace hfs
{
class http_request
{
public:
    http_request();

    explicit http_request(const std::string &buf);

    explicit http_request(const char *buf, size_t len);

    http_request(
        const std::string &method, const std::string &path,
        const std::string &version, const std::string &body,
        const std::unordered_map<std::string, std::string> &headers
    );

    ~http_request();

    /**
     * @brief Retrieve the HTTP status after parsing the request.
     *
     * If there is an error during the parsing process, the status will be set
     * to `4xx` HTTP status code, accordingly to the error. By default, the
     * status code is set to `200 (HTTP_STATUS_OK)`.
     *
     * However, the status code is not final and can be changed during the
     * response preparation. For example, if the server has an internal server
     * problem, the status code will be set to `5xx`.
     *
     * @return `http_status_code_t`
     */
    http_status_code_t
    status() const noexcept;

    /**
     * @brief Retrieve the request path that is part of the request line.
     *
     * The request line is defined in RFC 2616 Section 5.1 as follow:
     *
     * ```
     * Request-Line = Method SP Request-URI SP HTTP-Version CRLF
     * ```
     *
     * #### For example:
     *
     * @code
     * ```cpp
     * std::string req = "GET /index.html HTTP/1.1\r\n"
     *                   "Host: localhost:7000\r\n"
     *                  "User-Agent: curl/7.68.0\r\n";
     * hfs::http_request request(req);
     * std::cout << request.method() << std::endl; // GET
     * ```
     * @endcode
     *
     * @return `const std::string&`
     */
    const std::string &
    method() const noexcept;

    /**
     * @brief Retrieve the request path that is part of the request line.
     *
     * The request line is defined in RFC 2616 Section 5.1 as follow:
     *
     * ```
     * Request-Line = Method SP Request-URI SP HTTP-Version CRLF
     * ```
     *
     * #### For example:
     *
     * @code
     * ```cpp
     * std::string req = "GET /index.html HTTP/1.1\r\n"
     *                   "Host: localhost:7000\r\n"
     *                  "User-Agent: curl/7.68.0\r\n";
     * hfs::http_request request(req);
     * std::cout << request.path() << std::endl; // /index.html
     * ```
     * @endcode
     *
     * @return `const std::string&`
     */
    const std::string &
    path() const noexcept;

    /**
     * @brief Retrieve the HTTP version that is part of the request line.
     *
     * The request line is defined in RFC 2616 Section 5.1 as follow:
     *
     * ```
     * Request-Line = Method SP Request-URI SP HTTP-Version CRLF
     * ```
     *
     * #### For example:
     *
     * @code
     * ```cpp
     * std::string req = "GET /index.html HTTP/1.1\r\n"
     *                   "Host: localhost:7000\r\n"
     *                  "User-Agent: curl/7.68.0\r\n";
     * hfs::http_request request(req);
     * std::cout << request.version() << std::endl; // HTTP/1.1
     * ```
     * @endcode
     *
     * @return `const std::string&`
     */
    const std::string &
    version() const noexcept;

    /**
     * @brief Retrieve the request body if there is any.
     *
     * The request line is defined in RFC 2616 Section 4.3 as follow:
     *
     * ```
     * message-body = entity-body
     *                | <entity-body encoded as per Transfer-Encoding>
     * ```
     *
     * #### For example:
     *
     * @code
     * ```cpp
     * std::string req = "POST /login.html HTTP/1.1\r\n"
     *                   "Host: localhost:7000\r\n"
     *                   "User-Agent: curl/7.68.0\r\n"
     *                   "Content-Length: 31\r\n"
     *                   "\r\n"
     *                   "username=admin&password=1234";
     * hfs::http_request request(req);
     * std::cout << request.version() << std::endl;
     * // username=admin&password=1234
     * ```
     * @endcode
     *
     * @return `const std::string&`
     */
    const std::string &
    body() const noexcept;

    /**
     * @brief Retrieve a header value by name.
     *
     * @param key Request header name.
     * @return `const std::string&`
     * @throw `std::out_of_range` - If the header is not found.
     */
    const std::string &
    header(const std::string &key) const;

    /**
     * @brief Print the request to the output stream.
     *
     * @param os Standard output stream.
     * @param req HTTP request object.
     * @return `std::ostream&`
     */
    friend std::ostream &
    operator<<(std::ostream &os, const http_request &req);

private:
    http_status_code_t __status;
    std::string __uuid;
    std::string __method;
    std::string __path;
    std::string __version;
    std::string __body;
    std::unordered_map<std::string, std::string> __headers;

    std::string __buf;

    void
    __parse();

    void
    __parse_request_line(const std::string &request_line);

    void
    __parse_headers(const std::string &header_line);
};
} // namespace hfs

#endif // __HTTP_REQUEST_H__
