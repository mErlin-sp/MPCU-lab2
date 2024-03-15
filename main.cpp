#include <iostream>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>

void print_usage() {
    std::cout << "Usage: ./program -a <address> -p <port> [-n] [-s] [-4] [-6]\n";
}

int main(int argc, char *argv[]) {
    int opt;
    char const *address = nullptr;
    char const *port = nullptr;
    bool print_name = false;
    bool print_service = false;
    bool ipv6 = false;

    while ((opt = getopt(argc, argv, "a:p:ns46")) != -1) {
        switch (opt) {
            case 'a':
                address = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'n':
                print_name = true;
                break;
            case 's':
                print_service = true;
                break;
            case '4':
                ipv6 = false;
                break;
            case '6':
                ipv6 = true;
                break;
            default:
                print_usage();
                return 1;
        }
    }

    if (address == nullptr && port == nullptr) {
        print_usage();
        return 1;
    }

    addrinfo hints{};
    addrinfo *result;
    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = ipv6 ? AF_INET6 : AF_INET; //AF_UNSPEC
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(address, port, &hints, &result) != 0) {
        std::cerr << "Error getting address info\n";
        return 1;
    }

    for (auto *rp = result; rp != nullptr; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET) {
            auto const *addr_in = reinterpret_cast<sockaddr_in *>(rp->ai_addr);
            if (address != nullptr) {
                std::cout << "IPv4 Address: " << inet_ntoa(addr_in->sin_addr) << "\n";
            }
            if (port != nullptr) {
                std::cout << "Port: " << addr_in->sin_port << "\n";
            }
        } else if (rp->ai_family == AF_INET6) {
            auto const *addr_in6 = reinterpret_cast<sockaddr_in6 *>(rp->ai_addr);
            if (address != nullptr) {
                char buffer[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, &addr_in6->sin6_addr, buffer, INET6_ADDRSTRLEN);
                std::cout << "IPv6 Address: " << buffer << "\n";
            }
            if (port != nullptr) {
                std::cout << "Port: " << addr_in6->sin6_port << "\n";
            }
        }

        if (address != nullptr && print_name) {
            char host[NI_MAXHOST];
            if (getnameinfo(rp->ai_addr, rp->ai_addrlen, host, NI_MAXHOST, nullptr, 0, 0) == 0) {
                std::cout << "Host name: " << host << "\n";
            }
        }

        if (port != nullptr && print_service) {
            char serv[NI_MAXSERV];
            if (getnameinfo(rp->ai_addr, rp->ai_addrlen, nullptr, 0, serv, NI_MAXSERV, NI_NUMERICSERV) == 0) {
                std::cout << "Service name: " << serv << "\n";
            }
        }
    }

    freeaddrinfo(result);

    return 0;
}
