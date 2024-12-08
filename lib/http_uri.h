#ifndef __HTTP_URI_H__
#define __HTTP_URI_H__ 1

#include <http_core.h>

namespace hfs
{
class http_uri
{
public:
    /**
     * @brief Construct a new http_uri object
     *
     * @param uri - The raw URI string that suffices the RFC 3986 standard.
     */
    http_uri();

    /**
     * @brief Construct a new http_uri object based on the given URI string.
     *
     * @param uri - The raw URI string that suffices the RFC 3986 standard.
     */
    explicit http_uri(std::string_view uri);

    /**
     * @brief Destroy the http_uri object
     */
    ~http_uri();

    /**
     * @brief Static method to parse the given URI based on uriparser.
     *
     * @param uri - The raw URI string that suffices the RFC 3986 standard.
     * @return UriUriA* - The parsed URI structure of uriparser.
     */
    static UriUriA *
    parse(std::string_view uri);

    /**
     * @brief Static method to split and return the path segments of the URI
     * for iteration.
     *
     * @param uri - The raw URI string that suffices the RFC 3986 standard.
     * @return std::vector<std::string>
     */
    static std::vector<std::string>
    split_path(std::string_view uri);

    /**
     * @brief Return the string representation of the raw URI.
     *
     * @return std::string_view
     */
    std::string_view
    uri() const noexcept;

    /**
     * @brief Return the string representation of the path segments of the URI.
     * If the path is empty, then return `/`. If there are multiple segments,
     * then return the segments separated by `/`.
     *
     * For example:
     *
     * @code
     * ```cpp
     * http_uri uri("http://localhost:8080/index.html");
     *
     * std::cout << uri.path() << std::endl; // Output: /index.html
     * ```
     * @endcode
     *
     * @return std::string_view
     *
     */
    std::string_view
    path() const noexcept;

    /**
     * @brief Retrieve the query parameter based on the given key. If the key
     * does not exist, then return an empty string.
     *
     * For example:
     *
     * @code
     * ```cpp
     * http_uri uri("http://localhost:8080/index.html?name=John&age=25");
     *
     * std::cout << uri.query("name") << std::endl; // Output: John
     * std::cout << uri.query("age") << std::endl;  // Output: 25
     * std::cout << uri.query("city") << std::endl; // Output: (empty)
     * ```
     * @endcode
     *
     * @param key - The query parameter key.
     * @return std::string_view
     */
    std::string_view
    query(std::string_view key) const noexcept;

    /**
     * @brief Return the fragment of the URI.
     *
     * For example:
     *
     * @code
     * ```cpp
     * http_uri uri("http://localhost:8080/index.html#section-1");
     *
     * std::cout << uri.fragment() << std::endl; // Output: section-1
     * ```
     * @endcode
     *
     * @return std::string_view
     */
    std::string_view
    fragment() const noexcept;

    /**
     * @brief Return the scheme of the URI.current_date
     *
     * For example:
     *
     * @code
     * ```cpp
     * http_uri uri("http://localhost:8080/index.html");
     *
     * std::cout << uri.scheme() << std::endl; // Output: http
     * ```
     * @endcode
     *
     * @return std::string_view
     */
    std::string_view
    scheme() const noexcept;

    /**
     * @brief Return the host of the URI.
     *
     * For example:
     *
     * @code
     * ```cpp
     * http_uri uri("http://localhost:8080/index.html");
     *
     * std::cout << uri.host() << std::endl; // Output: localhost
     * ```
     * @endcode
     *
     * @return std::string_view
     */
    std::string_view
    host() const noexcept;

    /**
     * @brief Return the port of the URI.
     *
     * For example:
     *
     * @code
     * ```cpp
     * http_uri uri("http://localhost:8080/index.html");
     *
     * std::cout << uri.port() << std::endl; // Output: 8080
     * ```
     * @endcode
     *
     * @return std::string_view
     */
    std::string_view
    port() const noexcept;

private:
    std::string __uri;
    std::vector<std::string> __path;
    std::unordered_map<std::string, std::string> __query;
    std::string __fragment;
    std::string __scheme;
    std::string __host;
    std::string __port;
};
} // namespace hfs

#endif // __HTTP_URI_H__
