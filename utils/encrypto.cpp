#include "encrypto.h"

RSAClient::RSAClient()
{
    RSA* keypair{ RSA_generate_key(KEY_LENGTH, RSA_3, nullptr, nullptr) };
    BIO* pri{ BIO_new(BIO_s_mem()) };
    BIO* pub{ BIO_new(BIO_s_mem()) };

    PEM_write_bio_RSAPrivateKey(pri, keypair, nullptr, nullptr, 0, nullptr, nullptr);
    PEM_write_bio_RSA_PUBKEY(pub, keypair);
    size_t pri_len = BIO_pending(pri);
    size_t pub_len = BIO_pending(pub);
    char* pri_key{ new char[pri_len + 1]{ 0 } };
    char* pub_key{ new char[pub_len + 1]{ 0 } };

    BIO_read(pri, pri_key, pri_len);
    BIO_read(pub, pub_key, pub_len);
    private_key = pri_key;
    public_key = pub_key;

    RSA_free(keypair);
    BIO_free_all(pub);
    BIO_free_all(pri);
    delete[] pri_key;
    delete[] pub_key;
}

RSAClient::RSAClient(const std::string& pub_key)
    : private_key(std::string{ "" })
    , public_key(pub_key) { }

std::string RSAClient::private_key_encrypt(const std::string& origin_text) const
{
    std::string encrypt_text;
    BIO* keybio{ BIO_new_mem_buf((unsigned char*)private_key.c_str(), -1) };
    RSA* rsa{ RSA_new() };
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, nullptr, nullptr);
    if (!rsa) {
        spdlog::error("PEM read bio RSA PUBKEY Failed!");
        BIO_free_all(keybio);
        return std::string{ "" };
    }
    int len{ RSA_size(rsa) };
    char* text{ new char[len + 1]{ 0 } };
    int ret{RSA_private_encrypt(origin_text.size(),
                                (const unsigned char*)origin_text.c_str(),
                                (unsigned char*)text,
                                rsa,
                                RSA_PKCS1_PADDING)
    };
    if (ret >= 0) {
        encrypt_text = std::string(text, ret);
    }
    delete[] text;
    BIO_free_all(keybio);
    RSA_free(rsa);

    return encrypt_text;
}

std::string RSAClient::public_key_encrypt(const std::string& origin_text) const
{
    std::string encrypt_text;
    BIO* keybio{ BIO_new_mem_buf((unsigned char*)public_key.c_str(), -1) };
    RSA* rsa{ RSA_new() };
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, nullptr, nullptr);
    if (!rsa) {
        spdlog::error("PEM read bio RSA PUBKEY Failed!");
        BIO_free_all(keybio);
        return std::string{ "" };
    }
    int len{ RSA_size(rsa) };
    char* text{ new char[len + 1]{ 0 } };
    int ret{ RSA_public_encrypt(origin_text.size(),
                                (const unsigned char*)origin_text.c_str(),
                                (unsigned char*)text,
                                rsa,
                                RSA_PKCS1_PADDING)
    };
    if (ret >= 0) {
        encrypt_text = std::string(text, ret);
    }
    delete[] text;
    BIO_free_all(keybio);
    RSA_free(rsa);

    return encrypt_text;
}

std::string RSAClient::private_key_decrypt(const std::string& secret_text) const
{
    std::string decrypt_text;
    BIO* keybio{ BIO_new_mem_buf((unsigned char*)private_key.c_str(), -1) };
    RSA* rsa{ RSA_new() };

    rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, nullptr, nullptr);
    if (!rsa) {
        auto err{ ERR_get_error() };
        char err_msg[1024]{ 0 };
        ERR_error_string(err, err_msg);
        spdlog::error(err_msg);
        BIO_free_all(keybio);
        return std::string{ "" };
    }

    int len{ RSA_size(rsa) };
    char* text{ new char[len + 1]{ 0 } };
    int ret{RSA_private_decrypt(secret_text.size(),
                               (const unsigned char*)secret_text.c_str(),
                               (unsigned char*)text,
                               rsa,
                               RSA_PKCS1_PADDING)
    };
    if (ret >= 0) {
        decrypt_text = std::string(text, ret);
    }

    delete[] text;
    BIO_free_all(keybio);
    RSA_free(rsa);
    return decrypt_text;
}

std::string RSAClient::public_key_decrypt(const std::string& secret_text) const
{
    std::string decrypt_text;
    BIO* keybio{ BIO_new_mem_buf((unsigned char*)public_key.c_str(), -1) };
    RSA* rsa{ RSA_new() };

    rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, nullptr, nullptr);
    if (!rsa) {
        auto err{ ERR_get_error() };
        char err_msg[1024]{ 0 };
        ERR_error_string(err, err_msg);
        spdlog::error(err_msg);
        BIO_free_all(keybio);
        return std::string{ "" };
    }

    int len{ RSA_size(rsa) };
    char* text{ new char[len + 1]{ 0 } };
    int ret{RSA_public_decrypt(secret_text.size(),
                               (const unsigned char*)secret_text.c_str(),
                               (unsigned char*)text,
                               rsa,
                               RSA_PKCS1_PADDING)
    };
    if (ret >= 0) {
        decrypt_text = std::string(text, ret);
    }

    delete[] text;
    BIO_free_all(keybio);
    RSA_free(rsa);
    return decrypt_text;
}
