#ifndef QUICKIM_ENCRYPTO_H
#define QUICKIM_ENCRYPTO_H
#define OPENSSL_API_COMPAT 908
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <spdlog/spdlog.h>
#include <string>
#include <tuple>
constexpr int KEY_LENGTH{ 1024 };

class RSAClient
{
private:
    std::string private_key;
    std::string public_key;
public:
    RSAClient();
    RSAClient(const std::string& pub_key);
    RSAClient(const RSAClient&) = delete;
    ~RSAClient() = default;

    std::string private_key_encrypt(const std::string& origin_text) const;
    std::string public_key_encrypt(const std::string& origin_text) const;
    std::string private_key_decrypt(const std::string& secret_text) const;
    std::string public_key_decrypt(const std::string& secret_text) const;
};

class AESClient
{
private:
    std::string key;
public:
    AESClient();
    AESClient(const std::string& key);
    ~AESClient() = default;
};

#endif
