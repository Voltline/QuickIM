#ifndef QUICKIM_SERVER_H
#define QUICKIM_SERVER_H
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
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
#include <map>
#include "MessageQueue.h"
#include "MessageType.h"
#include "type.h"

class TCPServer
{
private:
    std::string ip;
    int port;
    int sock_fd;
    std::vector<int> client_fds;
    static std::map<int, pthread_t> thrs; // fd : thread_id
    static MessageQueue<MessageType> mq;
    static std::map<int, int> account; // id : fd
public:
    TCPServer() = default;
    TCPServer(const std::string& path);
    TCPServer(const TCPServer&) = delete;
    ~TCPServer();

    void accept_connection();
    static void* handle_client_request(void* client_fd);
    static void* start(void *);
};


#endif
