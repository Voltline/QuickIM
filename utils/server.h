#ifndef QUICKIM_SERVER_H
#define QUICKIM_SERVER_H
#include <nlohmann/json.hpp>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/un.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


class TCPServer
{
private:
    std::string ip;
    int port;
    int sock_fd;
    std::vector<int> client_fds;
public:
    TCPServer() = default;
    TCPServer(const std::string& path);
    TCPServer(const TCPServer&) = delete;
    ~TCPServer();

    void accept_connection();
    static void* handle_client_request(void* client_fd);
};


#endif
