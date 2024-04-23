cmake_minimum_required(VERSION 3.28)
project(QuickIM)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-lpthread -lsocket -lnsl)

include_directories(include)
include_directories(include/nlohmann)
include_directories(include/nlohmann/detail)
include_directories(include/nlohmann/detail/conversions)
include_directories(include/nlohmann/detail/input)
include_directories(include/nlohmann/detail/iterators)
include_directories(include/nlohmann/detail/meta)
include_directories(include/nlohmann/detail/meta/call_std)
include_directories(include/nlohmann/detail/output)
include_directories(include/nlohmann/thirdparty)
include_directories(include/nlohmann/thirdparty/hedley)

add_executable(QuickIM
        main.cpp
        utils/client.cpp
        utils/server.cpp)
