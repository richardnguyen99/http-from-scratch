#include "blocking_http_server.h"

namespace hfs
{

blocking_http_server::blocking_http_server() : __req(nullptr), __res(nullptr)
{
#ifdef DEBUG
    std::cout << "blocking_http_server::blocking_http_server()" << std::endl;
#endif

    this->__router           = hfs::http_router();
    this->__router.base_name = "/";
    this->__static_path      = "../public";
    this->__static_dir       = std::filesystem::directory_entry(
        std::filesystem::path(this->__static_path)
    );
}

blocking_http_server::~blocking_http_server()
{
#ifdef DEBUG
    std::cout << "blocking_http_server::~blocking_http_server()" << std::endl;
#endif
}

void
blocking_http_server::start()
{
#ifdef DEBUG
    std::cout << "blocking_http_server::start() - ";
#endif

    std::cout << "Server is listening on "
              << "http://localhost:" << this->__port << std::endl;

    // The server keeps running and accepting incoming connections until an
    // interrupt signal is received.
    for (;;)
    {
        // Client structucture to store the client address information
        struct sockaddr_storage client_addr;
        socklen_t client_addr_size = sizeof(client_addr);

        // Accept the incoming connection
        int client_socket = accept(
            this->__socket, (struct sockaddr *)&client_addr, &client_addr_size
        );
        if (client_socket == -1)
        {
            std::cerr << "accept: " << std::strerror(errno) << std::endl;
            continue;
        }

        // Retrieve the client IP address and port number
        char client_ip[INET6_ADDRSTRLEN];
        inet_ntop(
            client_addr.ss_family,
            &(((struct sockaddr_in *)&client_addr)->sin_addr), client_ip,
            sizeof(client_ip)
        );

#ifdef DEBUG
        std::cout << "Got a connection from " << client_ip << ":"
                  << ((struct sockaddr_in *)&client_addr)->sin_port
                  << std::endl;
#endif

        // Start reading the incoming request
        char buf[HTTP_BUFSZ + 1];
        ssize_t brecv, bsent;
        std::size_t total_recv, total_sent;
        char *body_ptr = nullptr, *endhdr_ptr = nullptr;

        std::memset(buf, 0, HTTP_BUFSZ);

        // Read everything from the client socket to the buffer.
        for (total_recv = 0; total_recv < HTTP_BUFSZ;)
        {
            // Read the request in a loop because the request may not be read
            // fully in a single read call.
            brecv = recv(client_socket, buf + total_recv, HTTP_BUFSZ, 0);

            handle_syscall_error(brecv, "recv");

            if (brecv == 0)
            {
                break;
            }

            total_recv += brecv;

            if (total_recv >= HTTP_BUFSZ)
            {
                break;
            }

            buf[total_recv] = '\0';

            // Check if the end of the header is reached and mark the start
            // of the body although it may not be read fully.
            if ((endhdr_ptr = std::strstr(buf, "\r\n\r\n")) != nullptr)
            {
                body_ptr = endhdr_ptr + 4;
                break;
            }
        }

        if (endhdr_ptr == nullptr && body_ptr == nullptr)
        {
            std::cerr << "http_request: Invalid request" << std::endl;
            handle_syscall_error(close(client_socket), "close");
            continue;
        }

#ifdef DEBUG
        std::cout << "Received " << total_recv << " bytes" << std::endl;
#endif

        try
        {
            this->__req = std::make_unique<http_request>(buf, total_recv);
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "http_request::constructor(buf, len): " << e.what()
                      << std::endl;

            handle_syscall_error(close(client_socket), "close");
            continue;
        }

        if (this->__req == nullptr)
        {
            std::cerr << "http_request: Failed to parse the request"
                      << std::endl;

            handle_syscall_error(close(client_socket), "close");
            continue;
        }

        // Perform check on the request
        if (this->__req->status() != HTTP_STATUS_OK)
        {
            std::cerr << "http_request: " << this->__req->status() << std::endl;

            handle_syscall_error(close(client_socket), "close");
            continue;
        }

        // If the request is a POST or PUT, parse the body
        if (this->__req->method() == "POST" || this->__req->method() == "PUT")
        {
            // Retrieve the content length
            try
            {
                auto it = this->__req->header("Content-Length");

                size_t content_length = std::stoul(it);

                // If the range between body_ptr and buf + total_recv is less
                // than the content length, then there are still data to read
                // from the client socket.

                size_t read_body = buf + total_recv - body_ptr;
                std::string final_body(body_ptr, read_body);

                if (read_body < content_length)
                {
                    // Decide how many more data to read

                    size_t body_recv;
                    size_t remaining = content_length - read_body;

                    // Allocate memory for the body

                    char *body_buf = new char[remaining + 1];

                    for (body_recv = 0; body_recv < remaining;)
                    {
                        brecv = recv(
                            client_socket, body_buf + body_recv, remaining, 0
                        );

                        handle_syscall_error(brecv, "recv");

                        if (brecv == 0)
                        {
                            break;
                        }

                        body_recv += brecv;
                        body_buf[body_recv] = '\0';
                    }

                    final_body.append(body_buf, body_recv);
                    this->__req->set_data(this->__req->data() + body_buf);
                    delete[] body_buf;
                }

                this->__req->set_body(final_body);
            }
            catch (const std::out_of_range &e)
            {
                std::cerr << "http_request::header(Content-Length): "
                          << e.what() << std::endl;

                handle_syscall_error(close(client_socket), "close");
                continue;
            }
        }

        this->__res =
            std::make_unique<http_response>(this->__static_path + "/pages");

        if (this->__res == nullptr)
        {
            std::cerr << "http_response: Failed to create a response"
                      << std::endl;

            handle_syscall_error(close(client_socket), "close");
            continue;
        }

        // Split the path by "/"
        std::vector<std::string> path_parts;
        std::string path = this->__req->path();
        std::istringstream path_stream;

        path_stream.str(path);

        for (std::string part; std::getline(path_stream, part, '/');)
        {
            if (!part.empty())
            {
                path_parts.push_back(part);
            }
            else
            {
                path_parts.push_back("/");
            }
        }

        // Find the handler for the path
        bool is_static                            = false;
        hfs::http_router *router                  = nullptr;
        hfs::http_router::route_handler_t handler = nullptr;

        for (const auto &part : path_parts)
        {
            if (router == nullptr || part == "/")
            {
                router = &this->__router;
            }
            else if (router->routes.find(part) != router->routes.end())
            {
                router = &router->routes[part];
                // Find in the static files
            }
            else
            {
                is_static = true;
            }
        }

        // Retrieve the handler for the method
        if (is_static)
        {
            if (this->__server_static(client_socket) == -1)
                continue;
        }
        else
        {
            handler = router->handlers[this->__req->method()];
            if (handler == nullptr)
            {
                std::cerr << "No handler found for " << this->__req->method()
                          << " " << this->__req->path() << std::endl;
                close(client_socket);
                continue;
            }

            // Call the handler
            handler(*this->__req, *this->__res);

            this->__res->header(
                "Cache-Control", "private, must-revalidate, max-age=0"
            );
        }

        // Prepare response header for server
        this->__res
            ->header(
                "Server", std::string(hfs::HTTP_SERVER_NAME) + "/" +
                              std::string(hfs::HTTP_SERVER_VERSION)
            )
            .header("Connection", "close")
            .header("X-Request-ID", this->__req->uuid());

        std::string response  = (*this->__res)();
        const char *resp_buf  = response.c_str();
        const size_t resp_len = response.length();

        // Send the response
        for (bsent = 0, total_sent = 0; total_sent < resp_len;)
        {
            bsent = send(client_socket, resp_buf + bsent, resp_len - bsent, 0);

            if (bsent == -1)
            {
                std::cerr << "send: " << std::strerror(errno) << std::endl;
                break;
            }

            total_sent += (size_t)bsent;
        }

#ifdef DEBUG
        std::cout << *(this->__req);
        std::cout << "Sent " << total_sent << " bytes" << std::endl;
#endif
        close(client_socket);
        this->__req.reset(nullptr);
        this->__res.reset(nullptr);
    }
}

void
blocking_http_server::listen(int port, int backlog, int optval)
{
    int ret;
    struct addrinfo *server_info;

#ifdef DEBUG
    std::cout << "blocking_http_server::listen(port = " << port
              << ", backlog = " << backlog << ", optval = " << optval << " )"
              << std::endl;
#endif

    std::memset(&this->__hints, 0, sizeof(struct addrinfo));
    this->__port        = port;
    this->__socket_flag = optval;
    this->__backlog     = backlog;

    // Prepare server info
    this->__hints.ai_family   = AF_UNSPEC;   // IPv4 or IPv6
    this->__hints.ai_socktype = SOCK_STREAM; // Use TCP
    this->__hints.ai_flags    = AI_PASSIVE;  // Make the socket ready for server
    this->__hints.ai_protocol = 0;           // Any protocol

#ifdef DEBUG
    std::cout << "├── construct struct addrinfo for server" << std::endl;
#endif

    // Find address information for the server socket
    if ((ret = getaddrinfo(
             nullptr, std::to_string(port).c_str(), &this->__hints, &server_info
         )) != 0)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(ret) << std::endl;
        return;
    }

#ifdef DEBUG
    std::cout << "├── finding address information for server" << std::endl;
#endif

    // Iterate through the server info and create a socket for the server
    for (struct addrinfo *p = server_info; p != nullptr; p = p->ai_next)
    {

#ifdef DEBUG
        std::cout << "│   ├── create a socket for the server" << std::endl;
#endif
        if ((this->__socket =
                 socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            std::cerr << "socket: " << std::strerror(errno) << std::endl;
            continue;
        }

        this->__socket_flag = 1;

        // Set the socket option to reuse the address
        if (setsockopt(
                this->__socket, SOL_SOCKET, SO_REUSEADDR, &this->__socket_flag,
                sizeof(int)
            ) == -1)
        {
            std::cerr << "setsockopt: " << std::strerror(errno) << std::endl;
            close(this->__socket);
            continue;
        }

        // Bind the socket to listen for incoming connections
        if (bind(this->__socket, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(this->__socket);
            std::cerr << "bind: " << std::strerror(errno) << std::endl;
            continue;
        }

#ifdef DEBUG
        std::cout << "\033[F";
        std::cout << "│   └── create a socket for the server" << std::endl;
#endif

        break;
    }

    freeaddrinfo(server_info);

    // Use :: to reference to the global namespace, which `listen` from the
    // `sys/socket.h` header file locates.

#ifdef DEBUG
    std::cout << "└── bind the socket for listening to incoming requests"
              << std::endl;
#endif
    if (::listen(this->__socket, this->__backlog) == -1)
    {
        std::cerr << "listen: " << std::strerror(errno) << std::endl;
        return;
    }

    // Initialize the environment for the HTTP response
    hfs::http_response::env.set_include_callback(
        [this](const std::string &path, const std::string &template_name)
            -> inja::Template
        {
            std::cout << "include_callback(" << path << ", " << template_name
                      << ")" << std::endl;

            std::string template_path =
                this->__static_path + "/pages/" + path + template_name;

            int fd;
            struct stat st;
            char *buffer;

            if ((fd = open(template_path.c_str(), O_RDONLY)) == -1)
            {
                if (errno == ENOENT)
                {
                    throw std::runtime_error(
                        "inja::Template::env::set_include_callback not "
                        "found: " +
                        template_name + " (Full path: " + template_path + ")."
                    );
                }
                else
                {
                    throw std::runtime_error(
                        "inja::Template::env::set_include_callback internal "
                        "error : " +
                        template_name + " (Reason: " + strerror(errno) + ")."
                    );
                }
            }

            if (fstat(fd, &st) == -1)
            {
                close(fd);
                throw std::runtime_error(
                    "inja::Template::env::set_include_callback internal "
                    "error : " +
                    template_name + " (Reason: " + strerror(errno) + ")."
                );
            }

            buffer = (char *)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

            if (buffer == MAP_FAILED)
            {
                close(fd);
                throw std::runtime_error(
                    "inja::Template::env::set_include_callback internal "
                    "error : " +
                    template_name + " (Reason: MAP_FAILED)."
                );
            }

            if (close(fd) == -1)
            {
                munmap(buffer, st.st_size);
                throw std::runtime_error(
                    "inja::Template::env::set_include_callback internal "
                    "error : " +
                    template_name + " (Reason: " + strerror(errno) + ")."
                );
            }

            std::string temp(buffer, st.st_size);

            if (munmap(buffer, st.st_size) == -1)
            {
                throw std::runtime_error(
                    "inja::Template::env::set_include_callback internal "
                    "error : " +
                    template_name + " (Reason: " + strerror(errno) + ")."
                );
            }

            return hfs::http_response::env.parse(temp);
        }
    );
}

void
blocking_http_server::register_handler(
    const std::string &path, const std::string &method,
    hfs::http_router::route_handler_t handler
)
{
    std::cout << "blocking_http_server::register_handler(" << path << ", "
              << method << ")" << std::endl;

    if (path[0] != '/')
    {
        throw std::runtime_error(
            "Invalid path: " + path + " (must start with /)"
        );
    }

    if (this->__router.handlers.find(method) == this->__router.handlers.end())
    {
        throw std::runtime_error("Invalid HTTP method");
    }

    if (path == "/")
    {
        // Register the handler with the router
        this->__router.handlers[method] = handler;

        // Register the path with the router
        this->__router.routes[path] = hfs::http_router();
    }
    else
    {
        // Split the path by "/"
        std::vector<std::string> path_parts;
        std::istringstream path_stream;

        path_stream.str(path);

        for (std::string part; std::getline(path_stream, part, '/');)
        {
            if (!part.empty())
            {
                path_parts.push_back(part);
            }
        }

        // Register the handler with the router
        hfs::http_router *router = &this->__router;

        for (const auto &part : path_parts)
        {
            if (router->routes.find(part) == router->routes.end())
            {
                router->routes[part] = hfs::http_router();
            }

            router = &router->routes[part];
        }

        router->handlers[method] = handler;
    }
}

void
blocking_http_server::register_static_handler(const std::string &path)
{
    std::filesystem::path static_dir(path);
    if (!std::filesystem::exists(static_dir))
    {
        throw std::runtime_error("Directory does not exist");
    }

    this->__static_path = path;
    this->__static_dir  = std::filesystem::directory_entry(static_dir);
}

void
blocking_http_server::register_error_handler(
    hfs::http_status_code_t status_code, const std::string &path,
    hfs::http_router::route_handler_t handler
)
{
    (void)status_code;
    (void)path;
    (void)handler;
}

void
blocking_http_server::handle_error(
    int client_socket, hfs::http_status_code_t status_code,
    const std::string &reason
)
{
    if (this->__req == nullptr)
    {
        handle_syscall_error(close(client_socket), "close");
        return;
    }

    if (this->__res == nullptr)
    {
        this->__res = std::make_unique<http_response>(this->__static_path);
    }

    std::string path = this->__req->path();

    // Split the path by "/"
    std::vector<std::string> path_parts;
    std::istringstream path_stream;

    path_stream.str(path);

    for (std::string part; std::getline(path_stream, part, '/');)
    {
        if (!part.empty())
        {
            path_parts.push_back(part);
        }
        else
        {
            path_parts.push_back("/");
        }
    }

    // Find the handler for the path
    hfs::http_router::error_handler_t handler = nullptr;

    for (const auto &part : path_parts)
    {
        if (this->__router.routes.find(part) != this->__router.routes.end())
        {
            handler = this->__router.routes[part].error_handlers[status_code];
        }
    }

    if (handler == nullptr)
    {
        handler = hfs::http_router::default_error_handler;
    }

    handler(status_code, reason, *this->__req, *this->__res);
}

int
blocking_http_server::__server_static(int client_socket)
{
    int fd;
    struct stat file_stat;
    char *body            = nullptr;
    std::string file_path = this->__static_path + this->__req->path();

    if ((fd = open(file_path.c_str(), O_RDONLY)) == -1)
    {
        if (errno == ENOENT)
        {
            this->handle_error(
                client_socket, HTTP_STATUS_NOT_FOUND,
                "Path not found: " + this->__req->path()
            );

            return 0;
        }

        handle_syscall_error(fd, "open");
    }

    handle_syscall_error(fstat(fd, &file_stat), "fstat");

    body =
        (char *)mmap(nullptr, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (body == MAP_FAILED)
    {
        std::cerr << "mmap: " << std::strerror(errno) << std::endl;
        close(fd);
        close(client_socket);
        return -1;
    }

    handle_syscall_error(
        [&body]() { return body == MAP_FAILED ? -1 : 0; }(), "mmap"
    );

    handle_syscall_error(close(fd), "close");

    std::string ext = file_path.substr(file_path.find_last_of(".") + 1);

    this->__res->status(HTTP_STATUS_OK)
        .header("Content-Type", hfs::http_mime(ext))
        .header("Cache-Control", "public, max-age=31536000")
        .header("Last-Modified", hfs::format_date(file_stat.st_mtime))
        .header("ETag", hfs::etag(file_stat.st_mtime, file_stat.st_size))
        .body(body);

    handle_syscall_error(munmap(body, file_stat.st_size), "munmap");

    return 0;
}

} // namespace hfs
