#include "client.h"

TCPClient::TCPClient(const std::string& path)
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
    sockfd = -1;
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
        throw std::runtime_error{ "[Error]: Failed creating socket" };
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error{ "[Error]: Failed connecting to server" };
    }
}

void TCPClient::send_msg(const std::string& msg)
{
    ssize_t bytes_sent{ send(sockfd, msg.c_str(), msg.size(), 0) };
    if (bytes_sent < 0) {
        throw std::runtime_error{ "[Error]: Failed sending msg to server" };
    }

    char buffer[40960]{ 0 };
    ssize_t bytes_received{ recv(sockfd, buffer, sizeof(buffer), 0) };
    if (bytes_received < 0) {
        throw std::runtime_error{ "[Error]: Failed receiving msg from server" };
    }

    buffer[bytes_received] = 0;
    std::cout << "[Response]: " << buffer << std::endl;
}


