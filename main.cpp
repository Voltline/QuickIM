#include "utils/client.h"
#include "utils/server.h"
#include <iostream>
#include <cstring>
#include <pthread.h>
using namespace std;
using json = nlohmann::json;

void terminate_handle()
{
    try {
        throw;
    }
    catch (std::exception& e) {
        spdlog::error(e.what());
    }
    catch (...) {
        spdlog::error("Unknown exception caught");
    }
    std::exit(EXIT_FAILURE);
}

int main(int argc, const char* argv[])
{
    std::set_terminate(terminate_handle);
    if (argc == 3) {
        TCPClient client{ "../config/config.json", stoi(argv[1]), argv[2] };
        client.start();
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