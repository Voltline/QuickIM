#include "client.h"
#include <sstream>

bool TCPClient::just_connect = true;

TCPClient::TCPClient(const std::string& path, int id)
{
    // 使用std::ifstream读取json内容
    just_connect = true;
    std::ifstream config_file{ path };
    if (!config_file.is_open()) {
        throw std::runtime_error{ "Failed opening configuration file" };
    }
    nlohmann::json config;
    config_file >> config;
    ip = config["ip"];
    port = config["port"];
    sockfd = -1;
    iid = id;
    connect_server();
}

TCPClient::~TCPClient()
{
    close(sockfd);
}

void TCPClient::connect_server()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error{ "Failed creating socket" };
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error{ "Failed connecting to server" };
    }
    send_msg(-1, std::to_string(iid));
}

void TCPClient::send_msg(int id, const std::string& msg)
{
    nlohmann::json json_obj;
    json_obj["from"] = iid;
    json_obj["to"] = id;
    json_obj["msg"] = msg;
    std::string to_send{ json_obj.dump() };
    ssize_t bytes_sent{ send(sockfd, to_send.c_str(), to_send.size(), 0) };
    if (bytes_sent < 0) {
        throw std::runtime_error{ "Failed sending msg to server" };
    }
}

void* TCPClient::recv_msg(void* fd)
{
    int recv_fd{ *(int*)fd };
    while (true) {
        char buffer[40960]{ 0 };
        ssize_t bytes_received{ recv(recv_fd, buffer, sizeof(buffer), 0) };
        if (bytes_received < 0) {
            throw std::runtime_error{ "Failed receiving msg from server" };
        }

        buffer[bytes_received] = 0;
        if (strcmp(buffer, "refused") == 0) {
            if (!just_connect) spdlog::warn("另一台设备正在登录");
            throw std::runtime_error{ "Server tells to exit" };
        }
        std::cout << "[Response]: " << buffer << std::endl;
        just_connect = false;
    }
    pthread_exit(nullptr);
}

void TCPClient::start()
{
    try {
        pthread_create(&recv_thr, nullptr, TCPClient::recv_msg, &sockfd);
        pthread_detach(recv_thr);
        bool connected{ true };
        while (connected) {
            std::cout << "输入您要发送消息的ID：" << std::endl;
            std::string s;
            std::getline(std::cin, s);
            int id{ stoi(s) };
            std::cout << "输入消息：" << std::endl;
            std::getline(std::cin, s);
            send_msg(id, s);
            if (s == "exit") {
                connected = false;
            }
        }
    }
    catch (std::exception& e) {
        spdlog::error("错误正在发生: {}", e.what());
    }
}



