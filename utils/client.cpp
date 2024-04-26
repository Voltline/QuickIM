#include "client.h"
#include <sstream>
#include <set>

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
    send_msg(Type::Login);
}

void TCPClient::send_msg(const Type& type, const nlohmann::json& id, const std::string& msg)
{
    /*  报文格式
     *  type为Type::Single/Multi/All时
     *  {
     *      "type": Type::Single/Multi/All,
     *      "from": iid,
     *      "to"  : [id],
     *      "msg" : "msg"
     *  }
     *  type为Type::Login时
     *  {
     *      "type": Type::Login,
     *      "from": iid
     *  }
     *  type为Type::Exit时
     *  {
     *      "type": Type::Exit,
     *      "from": iid
     *  }
     */
    nlohmann::json json_obj;
    json_obj["type"] = type;
    json_obj["from"] = iid;
    if (type != Type::Login || type != Type::Exit) {
        json_obj["to"] = id;
        json_obj["msg"] = msg;
    }
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
        if (strcmp(buffer, "") == 0) throw std::runtime_error{ "服务器已关闭" };
        std::cout << buffer << std::endl;
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
            std::cout << "输入ID(-2群发，多个空格分割的ID是向指定多人发送)：" << std::endl;
            std::string s1, s2;
            std::getline(std::cin, s1);
            std::istringstream sin{ s1 };
            std::cout << "输入消息：" << std::endl;
            std::getline(std::cin, s2);
            if (s2 == "exit") {
                connected = false;
                send_msg(Type::Exit);
            }
            else {
                int id{ 0 };
                nlohmann::json jarray{ nlohmann::json::array() };
                std::set<int> idset;
                while (sin >> id) {
                    idset.insert(id);
                }
                std::vector<int> ids(idset.begin(), idset.end());
                Type t;
                if (ids.size() == 1) {
                    if (ids[0] > 0) t = Type::Single;
                    else {
                        if (ids[0] == -2) t = Type::All;
                    }
                }
                else {
                    t = Type::Multi;
                }

                for (const auto& i : ids) {
                    if (i >= 0) jarray.push_back(i);
                }

                send_msg(t, jarray, s2);
            }
        }
    }
    catch (std::exception& e) {
        spdlog::error("错误正在发生: {}", e.what());
    }
}



