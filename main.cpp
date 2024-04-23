#include "utils/client.h"
#include "utils/server.h"
#include <iostream>
#include <cstring>
#include <pthread.h>
using namespace std;
using json = nlohmann::json;

int main(int argc, const char* argv[])
{
    if (argc == 3) {
        TCPClient client{ "../config/config.json", stoi(argv[2]) };
        bool connected{ true };
        while (connected) {
            cout << "输入您要发送消息的ID：";
            string s;
            getline(cin, s);
            int id{ stoi(s) };
            cout << "输入消息：";
            getline(cin, s);
            client.send_msg(id, s);
            if (s == "exit") {
                connected = false;
            }
        }
    }
    else {
        TCPServer server{ "../config/config.json" };
        pthread_t loop;
        pthread_create(&loop, nullptr, TCPServer::start, nullptr);
        pthread_detach(loop);
        server.accept_connection();
    }
    return 0;
}