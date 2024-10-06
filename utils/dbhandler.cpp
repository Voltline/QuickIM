#include "dbhandler.h"
#include <spdlog/spdlog.h>
#include <fstream>

dbhandler::dbhandler(const std::string& path)
    : ctx()
    , ssl_ctx(boost::asio::ssl::context::tls_client)
    , conn(ctx, ssl_ctx)
    , resolver(ctx.get_executor())
{
    std::ifstream fin{ path };
    if (!fin.is_open()) {
        throw std::runtime_error{ "Failed opening database configuration file" };
    }
    fin >> config;
    host = config["host"];
    port = config["port"];
    user = config["user"];
    password = config["password"];
    database = config["database"];
    endpoints = resolver.resolve(host, port);
    boost::mysql::handshake_params params{ user, password, database };
    conn.connect(*endpoints.begin(), params);

    auto sql{ "SELECT 'Hello';" };
    boost::mysql::results result;
    conn.execute(sql, result);
    std::string msg{ result.rows().at(0).at(0).as_string() };
    if (msg == "Hello") {
        spdlog::info("Database connected successfully");
    }
    else {
        spdlog::error("Database connection failed");
    }
}

dbhandler::~dbhandler()
{
    conn.close();
}