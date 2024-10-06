#ifndef DBHANDLER_H
#define DBHANDLER_H

#include <string>
#include <nlohmann/json.hpp>
#include <boost/mysql.hpp>

class dbhandler
{
private:
    nlohmann::json config;
    std::string host;
    std::string port;
    std::string user;
    std::string password;
    std::string database;
    boost::asio::io_context ctx;
    boost::asio::ssl::context ssl_ctx;
    boost::mysql::tcp_ssl_connection conn;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoints;
public:
    dbhandler(const std::string& path = "../config/dbconfig.json");
    ~dbhandler();
};



#endif
