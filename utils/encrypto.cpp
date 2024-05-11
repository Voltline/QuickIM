#include "encrypto.h"

RSAClient::RSAClient()
{
    rsaPublicKey = nullptr;
    rsaPrivateKey = nullptr;
    RSA* rsa = RSA_generate_key(KEY_LENGTH, RSA_F4, nullptr, nullptr);
    if (rsa != nullptr) {
        rsaPublicKey = RSAPublicKey_dup(rsa);
        rsaPrivateKey = RSAPrivateKey_dup(rsa);
        RSA_free(rsa);
    }
}

RSAClient::RSAClient(const std::string& publicKeyString)
{
    rsaPublicKey = nullptr;
    rsaPrivateKey = nullptr;

    BIO* bio = BIO_new_mem_buf(publicKeyString.c_str(), -1);
    rsaPublicKey = PEM_read_bio_RSAPublicKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
}

RSAClient::~RSAClient()
{
    if (rsaPublicKey != nullptr) {
        RSA_free(rsaPublicKey);
    }
    if (rsaPrivateKey != nullptr) {
        RSA_free(rsaPrivateKey);
    }
}

std::string RSAClient::encryptWithPublicKey(const std::string& plaintext)
{
    if (rsaPublicKey == nullptr) {
        return "";
    }

    int rsaSize = RSA_size(rsaPublicKey);
    unsigned char* ciphertext = new unsigned char[rsaSize];
    int result = RSA_public_encrypt(plaintext.length(),
                                    reinterpret_cast<const unsigned char*>(plaintext.c_str()),
                                    ciphertext,
                                    rsaPublicKey,
                                    RSA_PKCS1_PADDING);
    if (result == -1) {
        delete[] ciphertext;
        return "";
    }

    std::string encryptedText(reinterpret_cast<char*>(ciphertext), result);
    delete[] ciphertext;
    return encryptedText;
}

std::string RSAClient::decryptWithPrivateKey(const std::string& ciphertext)
{
    if (rsaPrivateKey == nullptr) {
        return "";
    }

    int rsaSize = RSA_size(rsaPrivateKey);
    unsigned char* decryptedText = new unsigned char[rsaSize];
    int result = RSA_private_decrypt(ciphertext.length(),
                                     reinterpret_cast<const unsigned char*>(ciphertext.c_str()),
                                     decryptedText,
                                     rsaPrivateKey,
                                     RSA_PKCS1_PADDING);
    if (result == -1) {
        delete[] decryptedText;
        return "";
    }

    std::string decrypted(reinterpret_cast<char*>(decryptedText), result);
    delete[] decryptedText;
    return decrypted;
}

std::string RSAClient::encryptWithPrivateKey(const std::string& plaintext)
{
    if (rsaPrivateKey == nullptr) {
        return "";
    }

    int rsaSize = RSA_size(rsaPrivateKey);
    unsigned char* ciphertext = new unsigned char[rsaSize];
    int result = RSA_private_encrypt(plaintext.length(),
                                     reinterpret_cast<const unsigned char*>(plaintext.c_str()),
                                     ciphertext,
                                     rsaPrivateKey,
                                     RSA_PKCS1_PADDING);
    if (result == -1) {
        delete[] ciphertext;
        return "";
    }

    std::string encryptedText(reinterpret_cast<char*>(ciphertext), result);
    delete[] ciphertext;
    return encryptedText;
}

std::string RSAClient::decryptWithPublicKey(const std::string& ciphertext)
{
    if (rsaPublicKey == nullptr) {
        return "";
    }

    int rsaSize = RSA_size(rsaPublicKey);
    unsigned char* decryptedText = new unsigned char[rsaSize];
    int result = RSA_public_decrypt(ciphertext.length(),
                                    reinterpret_cast<const unsigned char*>(ciphertext.c_str()),
                                    decryptedText,
                                    rsaPublicKey,
                                    RSA_PKCS1_PADDING);
    if (result == -1) {
        delete[] decryptedText;
        return "";
    }

    std::string decrypted(reinterpret_cast<char*>(decryptedText), result);
    delete[] decryptedText;
    return decrypted;
}

std::string RSAClient::getPublicKeyString() const
{
    if (rsaPublicKey == nullptr) {
        return "";
    }

    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPublicKey(bio, rsaPublicKey);
    BUF_MEM* mem = nullptr;
    BIO_get_mem_ptr(bio, &mem);
    std::string keyString(mem->data, mem->length);
    BIO_free_all(bio);
    return keyString;
}

std::string RSAClient::getPrivateKeyString() const
{
    if (rsaPrivateKey == nullptr) {
        return "";
    }

    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(bio, rsaPrivateKey, nullptr, nullptr, 0, nullptr, nullptr);
    BUF_MEM* mem = nullptr;
    BIO_get_mem_ptr(bio, &mem);
    std::string keyString(mem->data, mem->length);
    BIO_free_all(bio);
    return keyString;
}

AESClient::AESClient()
{
    generateRandomIV();
}

AESClient::AESClient(const std::string& key, const std::string& initializationVector)
{
    if (key.length() != AES_BLOCK_SIZE || initializationVector.length() != AES_BLOCK_SIZE) {
        // 错误处理：密钥和IV的长度不正确
        spdlog::error("Error Length of key or IV length");
    }

    memcpy(iv, initializationVector.c_str(), AES_BLOCK_SIZE);
    if (AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(key.c_str()), 128, &aesKey) < 0) {
        // 错误处理：设置加密密钥失败
        spdlog::error("Failed to set key");
    }
}

void AESClient::generateRandomIV()
{
    memset(iv, 0, AES_BLOCK_SIZE);
    if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
        // 错误处理：生成随机IV失败
        spdlog::error("Failed to generate random IV");
    }
}

std::string AESClient::encrypt(const std::string& plaintext)
{
    if (AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(iv), 128, &aesKey) < 0) {
        // 错误处理：设置加密密钥失败
        spdlog::error("Failed to set key");
    }

    int ciphertextLength = plaintext.length() + AES_BLOCK_SIZE;
    unsigned char* ciphertext = new unsigned char[ciphertextLength];
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(plaintext.c_str()),
                    ciphertext,
                    plaintext.length(),
                    &aesKey,
                    iv,
                    AES_ENCRYPT);

    std::string encryptedText(reinterpret_cast<char*>(ciphertext), ciphertextLength);
    delete[] ciphertext;
    return encryptedText;
}

std::string AESClient::decrypt(const std::string& ciphertext, const std::string& receivedIV)
{
    if (receivedIV.length() != AES_BLOCK_SIZE) {
        // 错误处理：接收到的IV长度不正确
        spdlog::error("Error length of IV");
    }

    memcpy(iv, receivedIV.c_str(), AES_BLOCK_SIZE);
    if (AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(iv), 128, &aesKey) < 0) {
        // 错误处理：设置解密密钥失败
        spdlog::error("Failed to set key");
    }

    unsigned char* decryptedText = new unsigned char[ciphertext.length()];
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(ciphertext.c_str()),
                    decryptedText,
                    ciphertext.length(),
                    &aesKey,
                    iv,
                    AES_DECRYPT);

    std::string decrypted(reinterpret_cast<char*>(decryptedText), ciphertext.length());
    delete[] decryptedText;
    return decrypted;
}
