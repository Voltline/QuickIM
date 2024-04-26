cmake_minimum_required(VERSION 3.28)
project(QuickIM)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-lpthread -lsocket -lnsl)

include_directories(include)
include_directories(include/spdlog)
include_directories(include/nlohmann)

add_executable(QuickIM
        main.cpp
        utils/client.cpp
        utils/server.cpp
)
