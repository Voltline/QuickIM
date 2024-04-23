#include "utils/client.h"
#include "utils/server.h"
#include <iostream>
#include <cstring>
using namespace std;
using json = nlohmann::json;

int main(int argc, const char* argv[])
{
    if (argc < 2 || strcmp(argv[1], "client") == 0) {
        TCPClient client{ "../config/config.json" };
        bool connected{ true };
        while (connected) {
            cout << "输入您要发送给服务器的内容：";
            string s;
            getline(cin, s);
            client.send_msg(s);
            if (s == "exit") {
                connected = false;
            }
        }
    }
    else {
        TCPServer server{ "../config/config.json" };
        server.accept_connection();
    }
    return 0;
}