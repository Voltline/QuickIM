#include "server.h"
#include <stdexcept>
#include <cstring>

MessageQueue<MessageType> TCPServer::mq = MessageQueue<MessageType>();
std::map<int, int> TCPServer::account = std::map<int, int>();

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
    for (auto tid : threads) {
        pthread_detach(tid);
    }
}

void TCPServer::accept_connection()
{
    try {
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
            threads.push_back(tid);
        }
    }
    catch (std::exception& e) {
        std::cout << "[Error accept]: " << e.what() << std::endl;
    }
}

void* TCPServer::handle_client_request(void* client_fd)
{
    int fd{ *(int*)client_fd };
    try {
        while (true) { // TODO: 客户端退出，服务端还没退出
            char buffer[40960]{ 0 };
            ssize_t bytes_received = recv(fd, buffer, sizeof(buffer), 0);
            if (bytes_received >= 0) {
                buffer[bytes_received] = 0;
                std::string buf{ buffer };
                std::cout << buf << std::endl;
                nlohmann::json j{ nlohmann::json::parse(buf) };
                int from_fd{ fd };
                std::string msg{ j["msg"] };
                if (msg == "exit") {
                    std::string response{ "Goodbye!" };
                    send(fd, response.c_str(), response.size(), 0);
                    std::cout << "[Info]: client" << fd << " exits" << std::endl;
                    TCPServer::account.erase(j["from"]);
                    close(fd);
                    break;
                }
                else {
                    TCPServer::mq.push(MessageType{ from_fd, j });
                }
            }
        }
    }
    catch (std::exception& e) {
        std::cout << "[Error handle]: " << e.what() << std::endl;
    }
    pthread_exit(nullptr);
}

void* TCPServer::start(void* p)
{
    std::string response;
    while (true) {
        auto task = TCPServer::mq.pop();
        int from{ task.from_fd };
        int from_id{ task.json["from"] }, to_id{ task.json["to"] };
        std::string msg{ task.json["msg"] };
        if (to_id == -1) {
            if (TCPServer::account[from_id] != 0) {
                response = "refused";
                send(from, response.c_str(), response.size(), 0);
                std::cout << "[Info]: client" << from << " exits" << std::endl;
                close(from);
            }
            else {
                TCPServer::account[from_id] = from;
                response = "Welcome to QuickIM, " + std::to_string(from_id);
                send(from, response.c_str(), response.size(), 0);
                std::cout << "[Info]: client" << from << " connects" << std::endl;
            }
        }
        if (TCPServer::account[to_id] == 0) {
            response = "[Warning]: User" + std::to_string(from_id) + " not exists";
            send(from, response.c_str(), response.size(), 0);
        }
        else {
            send(TCPServer::account[to_id], msg.c_str(), msg.size(), 0);
        }
    }
    pthread_exit(nullptr);
}