#include "blocking_http_server.h"

namespace hfs
{
blocking_http_server::blocking_http_server()
{
#ifdef DEBUG
    std::cout << "blocking_http_server::blocking_http_server()" << std::endl;
#endif
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
    std::cout << "blocking_http_server::start() - Server is listening on "
              << "http://localhost:" << this->__port << std::endl;

#endif

    for (;;)
    {
        struct sockaddr_storage client_addr;
        socklen_t client_addr_size = sizeof(client_addr);

        int client_socket = accept(
            this->__socket, (struct sockaddr *)&client_addr, &client_addr_size
        );

        if (client_socket == -1)
        {
            std::cerr << "accept: " << std::strerror(errno) << std::endl;
            continue;
        }

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

        char buf[HTTP_BUFSZ];
        ssize_t brecv, bsent;
        std::size_t total_recv, total_sent;

        std::memset(buf, 0, HTTP_BUFSZ);

        for (total_recv = 0; total_recv < HTTP_BUFSZ;)
        {
            brecv = recv(client_socket, buf, HTTP_BUFSZ, 0);

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

            if (buf[total_recv - 1] == '\n' && buf[total_recv - 2] == '\r')
            {
                break;
            }
        }

#ifdef DEBUG
        std::cout << "Received " << total_recv << " bytes" << std::endl;
#endif

        std::stringstream body;
        body << "Received: \n"
             << buf << "\n(Total " << total_recv << " bytes)\n";

        std::stringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: text/plain\r\n"
                 << "Content-Length: " << body.str().length() << "\r\n"
                 << "Connection: close\r\n"
                 << "\r\n"
                 << body.str();

        for (bsent = 0, total_sent = 0; total_sent < response.str().length();)
        {
            bsent = send(
                client_socket, response.str().c_str() + bsent,
                response.str().length() - bsent, 0
            );

            if (bsent == -1)
            {
                std::cerr << "send: " << std::strerror(errno) << std::endl;
                break;
            }

            total_sent += (size_t)bsent;
        }

#ifdef DEBUG
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

    this->__hints.ai_family   = AF_UNSPEC;   // IPv4 or IPv6
    this->__hints.ai_socktype = SOCK_STREAM; // Use TCP
    this->__hints.ai_flags    = AI_PASSIVE;  // Make the socket ready for server
    this->__hints.ai_protocol = 0;           // Any protocol

#ifdef DEBUG
    std::cout << "├── construct struct addrinfo for server" << std::endl;
#endif

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
    std::function<void()> handler
)
{
    std::cout << "blocking_http_server::register_handler(" << path << ", "
              << method << ")" << std::endl;

    handler();
}
} // namespace hfs
