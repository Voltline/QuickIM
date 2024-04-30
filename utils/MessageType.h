#ifndef QUICKIM_MESSAGETYEP_H
#define QUICKIM_MESSAGETYEP_H
#include <string>
#include <nlohmann/json.hpp>

class MessageType
{
public:
    int from_fd;
    nlohmann::json json;
};

#endif
