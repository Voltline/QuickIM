#include "server.h"
#include <stdexcept>
#include <cstring>

MessageQueue<MessageType> TCPServer::mq = MessageQueue<MessageType>();
std::map<int, int> TCPServer::account = std::map<int, int>();
std::map<int, pthread_t> TCPServer::thrs = std::map<int, pthread_t>();

TCPServer::TCPServer(const std::string& path)
{
    // 使用std::ifstream读取json内容
    std::ifstream config_file{ path };
    if (!config_file.is_open()) {
        throw std::runtime_error{ "Failed opening configuration file" };
    }
    nlohmann::json config;
    config_file >> config;
    ip = config["ip"];
    port = config["port"];

    // 初始化套接字
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        throw std::runtime_error{ "Failed creating socket" };
    }

    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    server_addr.sin_port = htons(port);

    // 绑定地址和端口
    if (bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error{ "Failed binding socket" };
    }

    // 开始监听连接
    if (listen(sock_fd, SOMAXCONN) < 0) {
        throw std::runtime_error{ "Failed listening on socket" };
    }
}

TCPServer::~TCPServer()
{
    close(sock_fd);
    for (auto client_fd : client_fds) {
        close(client_fd);
    }
    for (auto [fd, tid] : thrs) {
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
                throw std::runtime_error{ "Failed accepting client connection" };
            }
            client_fds.push_back(client_fd);

            // 处理客户端请求
            pthread_t tid;
            if (pthread_create(&tid, nullptr, handle_client_request, (void*)&client_fd)) {
                spdlog::warn("Failed creating thread with client_fd: {}", client_fd);
                close(client_fd);
                continue;
            }
            thrs[client_fd] = tid;
        }
    }
    catch (std::exception& e) {
        spdlog::error(e.what());
    }
}

void* TCPServer::handle_client_request(void* client_fd)
{
    int fd{ *(int*)client_fd };
    try {
        while (true) {
            char buffer[40960]{ 0 };
            ssize_t bytes_received = recv(fd, buffer, sizeof(buffer), 0);
            if (bytes_received >= 0) {
                buffer[bytes_received] = 0;
                std::string buf{ buffer };
                spdlog::info(buf);
                nlohmann::json j{ nlohmann::json::parse(buf) };
                Type t{ j["type"] };
                if (t == Type::Exit) {
                    std::string response{ "Goodbye!" };
                    send(fd, response.c_str(), response.size(), 0);
                    spdlog::info("client{} exits", fd);
                    TCPServer::account.erase(j["from"]);
                    close(fd);
                    pthread_cancel(thrs[fd]);
                    thrs.erase(fd);
                    break;
                }
                else {
                    TCPServer::mq.push(MessageType{ fd, j });
                }
            }
        }
    }
    catch (std::exception& e) {
        spdlog::error(e.what());
    }
    pthread_exit(nullptr);
}

void* TCPServer::start(void* p)
{
    std::string response;
    while (true) {
        auto task = TCPServer::mq.pop();
        Type t{ task.json["type"] };
        int from{ task.from_fd };
        if (t == Type::Login) {
            int from_id{ task.json["from"] };
            if (TCPServer::account[from_id] != 0) {
                int old_fd{ TCPServer::account[from_id] };
                response = "refused";
                send(old_fd, response.c_str(), response.size(), 0);
                spdlog::info("client{} re-login", from);
                close(old_fd);
                TCPServer::account.erase(from_id);
                pthread_cancel(thrs[old_fd]);
                thrs.erase(old_fd);
            }
            TCPServer::account[from_id] = from;
            response = "[Server] Welcome to QuickIM, User " + std::to_string(from_id);
            send(from, response.c_str(), response.size(), 0);
            spdlog::info("client{} connects", from);
        }
        else {
            int from_id{ task.json["from"] };
            nlohmann::json to_id{ task.json["to"] };
            std::string msg{ task.json["msg"] };
            if (task.json["type"] == Type::Single) {
                int to{ to_id[0] };
                if (TCPServer::account[to] == 0) {
                    response = "[Warning] User" + std::to_string(to) + " not exists";
                    send(from, response.c_str(), response.size(), 0);
                }
                else {
                    msg = "[From " + std::to_string(from_id) + "]: " + msg;
                    send(TCPServer::account[to], msg.c_str(), msg.size(), 0);
                }
            }
            else if (task.json["type"] == Type::Multi) {
                msg = "[From " + std::to_string(from_id) + "] " + msg;
                for (const auto & i : to_id) {
                    if (TCPServer::account[i] == 0) {
                        response = "[Warning] User" + std::to_string((int)i) + " not exists";
                        send(from, response.c_str(), response.size(), 0);
                    }
                    else {
                        send(TCPServer::account[i], msg.c_str(), msg.size(), 0);
                    }
                }
            }
            else {
                spdlog::info("client{} 发送了一条群发消息，内容为：{}", from_id, msg);
                msg = "[群发][From " + std::to_string(from_id) + "] " + msg;
                for (const auto& [user_id, user_fd] : account) {
                    send(user_fd, msg.c_str(), msg.size(), 0);
                }
            }
        }
    }
    pthread_exit(nullptr);
}