#ifndef QUICKIM_ENCRYPTO_H
#define QUICKIM_ENCRYPTO_H
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <spdlog/spdlog.h>
#include <string>
#include <tuple>
constexpr int KEY_LENGTH{ 1024 };

class RSAClient
{
private:
    RSA* rsaPublicKey;
    RSA* rsaPrivateKey;

public:
    RSAClient();
    RSAClient(const std::string& publicKeyString);
    ~RSAClient();

    std::string encryptWithPublicKey(const std::string& plaintext);
    std::string decryptWithPrivateKey(const std::string& ciphertext);

    std::string encryptWithPrivateKey(const std::string& plaintext);
    std::string decryptWithPublicKey(const std::string& ciphertext);

    std::string getPublicKeyString() const;
    std::string getPrivateKeyString() const;
};

class AESClient
{
private:
    AES_KEY aesKey;
    unsigned char iv[AES_BLOCK_SIZE];

public:
    AESClient();
    AESClient(const std::string& key, const std::string& initializationVector);
    ~AESClient() = default;

    std::string encrypt(const std::string& plaintext);
    std::string decrypt(const std::string& ciphertext, const std::string& receivedIV);

private:
    void generateRandomIV();
};

#endif
