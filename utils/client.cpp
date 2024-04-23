#include "client.h"
#include <sstream>

TCPClient::TCPClient(const std::string& path, int id)
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
        throw std::runtime_error{ "[Error]: Failed creating socket" };
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error{ "[Error]: Failed connecting to server" };
    }
    send_msg(-1, std::to_string(iid));
}

void TCPClient::send_msg(int id, const std::string& msg)
{
    nlohmann::json json_obj;
    json_obj["from"] = iid;
    json_obj["to"] = id;
    json_obj["msg"] = msg;
    std::cout << json_obj.dump() << std::endl;
    std::string to_send{ json_obj.dump() };
    std::cout << "to_send : " << to_send << std::endl;
    ssize_t bytes_sent{ send(sockfd, to_send.c_str(), to_send.size(), 0) };
    if (bytes_sent < 0) {
        throw std::runtime_error{ "[Error]: Failed sending msg to server" };
    }

    char buffer[40960]{ 0 };
    ssize_t bytes_received{ recv(sockfd, buffer, sizeof(buffer), 0) };
    if (bytes_received < 0) {
        throw std::runtime_error{ "[Error]: Failed receiving msg from server" };
    }

    buffer[bytes_received] = 0;
    if (strcmp(buffer, "refused") == 0) {
        throw std::runtime_error{ "[Exit]: Server tells to exit" };
    }
    std::cout << "[Response]: " << buffer << std::endl;
}


