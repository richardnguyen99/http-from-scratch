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
        char buf[HTTP_BUFSZ];
        ssize_t brecv, bsent;
        std::size_t total_recv, total_sent;

        std::memset(buf, 0, HTTP_BUFSZ);

        for (total_recv = 0; total_recv < HTTP_BUFSZ;)
        {
            // Read the request in a loop because the request may not be read
            // fully in a single read call.
            brecv = recv(client_socket, buf + total_recv, HTTP_BUFSZ, 0);

            if (brecv == -1)
            {
                std::cerr << "recv: " << std::strerror(errno) << std::endl;
                break;
            }

            if (brecv == 0)
            {
                break;
            }

            total_recv += brecv;
            buf[total_recv] = '\0';

            if (std::strstr(buf, "\r\n\r\n") != nullptr)
            {
                break;
            }
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

            close(client_socket);
            continue;
        }

        if (this->__req == nullptr)
        {
            std::cerr << "http_request: Failed to parse the request"
                      << std::endl;
            close(client_socket);
            continue;
        }

        this->__res = std::make_unique<http_response>();

        if (this->__res == nullptr)
        {
            std::cerr << "http_response: Failed to create a response"
                      << std::endl;
            close(client_socket);
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
            std::string file_path = this->__static_path + this->__req->path();
            std::filesystem::path file(file_path);

            if (!std::filesystem::exists(file))
            {
                std::cerr << "File not found: " << file_path << std::endl;
                close(client_socket);
                continue;
            }

            std::ifstream f(file_path);
            std::string body(
                (std::istreambuf_iterator<char>(f)),
                std::istreambuf_iterator<char>()
            );

            this->__res->status(HTTP_STATUS_OK)
                .header("Content-Type", "text/html")
                .header("Content-Length", std::to_string(body.length()))
                .header("X-Request-ID", this->__req->uuid())
                .header("Server", "HFS")
                .header("Connection", "close")
                .body(body);

            f.close();
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
        }

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
}

void
blocking_http_server::register_handler(
    const std::string &path, const std::string &method,
    hfs::http_router::route_handler_t handler
)
{
    std::cout << "blocking_http_server::register_handler(" << path << ", "
              << method << ")" << std::endl;

    if (this->__router.handlers.find(method) == this->__router.handlers.end())
    {
        throw std::runtime_error("Invalid HTTP method");
    }

    // Register the handler with the router
    this->__router.handlers[method] = handler;

    // Register the path with the router
    this->__router.routes[path] = hfs::http_router();
}

void
blocking_http_server::register_static(const std::string &path)
{
    std::filesystem::path static_dir(path);
    if (!std::filesystem::exists(static_dir))
    {
        throw std::runtime_error("Directory does not exist");
    }

    this->__static_path = path;
    this->__static_dir  = std::filesystem::directory_entry(static_dir);
}

} // namespace hfs
