#include "server.h"
#include <stdexcept>
#include <cstring>

TCPServer::TCPServer(const std::string& path)
{
    // 使用std::ifstream读取json内容
    std::ifstream config_file{ path };
    if (!config_file.is_open()) {
        throw std::runtime_error{ "[Error]: Failed opening configuration file" };
    }
    nlohmann::json config;
    config_file >> config;
    ip = config["ip"];
    port = config["port"];

    // 初始化套接字
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        throw std::runtime_error{ "[Error]: Failed creating socket" };
    }

    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    server_addr.sin_port = htons(port);

    // 绑定地址和端口
    if (bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error{ "[Error]: Failed binding socket" };
    }

    // 开始监听连接
    if (listen(sock_fd, SOMAXCONN) < 0) {
        throw std::runtime_error{ "[Error]: Failed listening on socket" };
    }
}

TCPServer::~TCPServer()
{
    close(sock_fd);
    for (auto client_fd : client_fds) {
        close(client_fd);
    }
}

void TCPServer::accept_connection()
{
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd{ accept(sock_fd, (struct sockaddr*)&client_addr, &client_len) };
        if (client_fd < 0) {
            throw std::runtime_error{ "[Error]: Failed accepting client connection" };
        }
        client_fds.push_back(client_fd);

        // 处理客户端请求
        pthread_t tid;
        if (pthread_create(&tid, nullptr, handle_client_request, (void*)&client_fd)) {
            std::cout << "[Warning]: Failed creating thread with client_fd : " << client_fd << std::endl;
            close(client_fd);
            continue;
        }
        pthread_detach(tid);
    }

    // handle_client_request(client_fd);
}

void* TCPServer::handle_client_request(void* client_fd)
{
    int fd{ *(int*)client_fd };
    while (true) {
        char buffer[40960]{ 0 };
        ssize_t bytes_received = recv(fd, buffer, sizeof(buffer), 0);
        if (bytes_received >= 0) {
            buffer[bytes_received] = 0;
            if (strcmp(buffer, "exit") != 0) {
                std::cout << "Received from client" << fd << ": " << buffer << std::endl;
                std::string response{ "Hello, this is from server!" };
                send(fd, response.c_str(), response.size(), 0);
            }
            else {
                std::string response{ "Goodbye!" };
                send(fd, response.c_str(), response.size(), 0);
                break;
            }
        }
    }
    pthread_exit(nullptr);
}