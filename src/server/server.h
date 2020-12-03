#ifndef SERVER_H_
#define SERVER_H_
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define MAX_CONNECTIONS 20

class Server {
public:
    // Class methods
    Server();
    void accept_connection();
    void* get_ip_address(sockaddr* sock);
    u_int16_t get_port_number(struct sockaddr* sock);

    // Class variables
    struct addrinfo inVal, *sv_info, *valid_info;

    struct sockaddr_storage client_addr;
    int accept_id;
    int address;
    int yes;

    char ip[INET6_ADDRSTRLEN];
    char ip1[INET6_ADDRSTRLEN];
    socklen_t addr_len;
};

#endif  // SERVER_H_