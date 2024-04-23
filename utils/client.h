#ifndef QUICKIM_CLIENT_H
#define QUICKIM_CLIENT_H
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


class TCPClient
{
private:
    std::string ip;
    int port;
    int sockfd;
    int iid;
public:
    TCPClient() = default;
    TCPClient(const std::string& path, int id);
    TCPClient(const TCPClient&) = delete;
    ~TCPClient();

    void connect_server();
    void send_msg(int id, const std::string& msg);
};


#endif
