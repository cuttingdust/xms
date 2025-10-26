#include "XSSL.h"

#include <iostream>

#include <openssl/err.h>
#include <openssl/ssl.h>

#ifndef _WIN32
#include <sys/socket.h>
#endif

class XSSL::PImpl
{
public:
    PImpl(XSSL *owenr);
    ~PImpl() = default;

public:
    XSSL *owenr_ = nullptr;
    SSL  *ssl_   = nullptr;
};

XSSL::PImpl::PImpl(XSSL *owenr) : owenr_(owenr)
{
}

XSSL::XSSL()
{
    std::cout << "XSSL create!" << std::endl;
    impl_ = std::make_unique<PImpl>(this);
}

XSSL::~XSSL()
{
    std::cout << "XSSL free!!!" << std::endl;
}

auto XSSL::set_ssl(SSL *ssl) const -> void
{
    impl_->ssl_ = ssl;
}

auto XSSL::get_ssl() const -> SSL *
{
    return impl_->ssl_;
}

auto XSSL::connect() -> bool
{
    if (!impl_->ssl_)
    {
        std::cout << "ssl == 0" << std::endl;
        return false;
    }
    /// 建立ssl连接验证，密钥协商
    int re = SSL_connect(impl_->ssl_);
    if (re <= 0)
    {
        std::cout << "SSL_connect failed!" << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }
    std::cout << "SSL_connect success!" << std::endl;
    printCipher();
    printCert();
    return true;
}

auto XSSL::isEmpty() const -> bool
{
    return impl_->ssl_ == nullptr;
}

auto XSSL::accept() const -> bool
{
    if (!impl_->ssl_)
    {
        return false;
    }
    /// 建立ssl连接验证，密钥协商
    int re = SSL_accept(impl_->ssl_);
    if (re <= 0)
    {
        std::cout << "XSSL::accept() failed!" << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }
    std::cout << "SSL_accept success!" << std::endl;
    printCipher();
    return true;
}

auto XSSL::printCipher() const -> void
{
    if (!impl_->ssl_)
    {
        return;
    }
    std::cout << SSL_get_cipher(impl_->ssl_) << std::endl;
}

auto XSSL::printCert() const -> void
{
    if (!impl_->ssl_)
    {
        return;
    }


    /// 获取到证书
    X509 *cert = SSL_get_peer_certificate(impl_->ssl_);
    if (!cert)
    {
        std::cout << "No certificate presented by peer!" << std::endl;
        return;
    }

    // X509_print_ex_fp(stdout, cert, 0, 0);
    // X509_free(cert);

    /*
Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number:
            2e:2e:a4:41:3e:f3:6d:cc:c9:c6:2b:b1:f5:8b:fa:63:1d:ee:57:a2
        Signature Algorithm: sha256WithRSAEncryption
        Issuer: C=CN, ST=zhejiang, L=hangzhou, O=TEST, OU=code, CN=127.0.0.1/emailAddress=123456798@163.com
        Validity
            Not Before: Apr  9 06:18:26 2025 GMT
            Not After : Apr  9 06:18:26 2026 GMT
        Subject: C=CN, ST=zhejiang, L=hanghzou, O=TEST, OU=code, CN=127.0.0.1/emailAddress=123456789@163.com
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                Public-Key: (2048 bit)
                Modulus:
                    00:cd:53:96:21:6b:84:05:74:00:02:d5:86:b8:22:
                    3a:1b:af:94:1d:06:ab:2c:7b:20:c5:23:28:c1:55:
                    07:bc:93:65:fa:47:c5:bc:8c:05:8c:81:ae:5d:d8:
                    90:63:ca:1d:48:15:f3:be:a9:55:bd:25:3f:cd:a1:
                    99:62:0d:79:5b:8c:1c:d7:80:a3:9a:c3:97:1a:7e:
                    8e:8a:2a:6a:cd:d1:d3:f9:e9:1f:21:50:c7:a8:75:
                    34:27:d7:31:23:73:55:e4:be:08:10:a1:70:19:32:
                    2f:d0:16:5b:22:69:cb:0f:3e:57:4d:57:20:b3:0b:
                    45:6b:f1:9f:ce:e1:08:d5:49:97:40:0f:f3:9a:62:
                    ff:d3:fe:3f:a9:69:7a:74:47:4c:a5:00:30:6d:77:
                    9f:f9:4d:c4:fc:ba:f2:14:4d:9e:97:28:0d:b0:73:
                    84:b7:4c:e2:d8:49:bc:5f:59:af:45:80:3b:0b:ee:
                    b3:61:9e:47:0f:3c:63:ea:b4:99:bf:9b:37:eb:c4:
                    83:3d:f5:f4:01:74:94:5d:4a:88:4f:e7:c5:fc:09:
                    b1:f6:dc:a3:dd:6b:40:62:e7:57:65:df:72:af:99:
                    06:57:69:1f:ce:d2:c5:a4:6f:09:a5:55:d0:17:5a:
                    1c:52:c7:f6:f8:0d:4f:a5:8c:7f:8c:b0:d8:76:07:
                    66:3d
                Exponent: 65537 (0x10001)
        X509v3 extensions:
            X509v3 Subject Key Identifier:
                A3:6C:83:25:06:D3:54:F7:90:1F:18:02:F7:60:77:23:2E:31:51:9D
    Signature Algorithm: sha256WithRSAEncryption
    Signature Value:
        4d:57:a5:5b:99:b1:d5:ca:73:f2:7d:0e:fd:0e:a0:80:1e:50:
        b9:30:c3:8e:b3:87:2d:9a:ae:1c:d8:be:4a:e0:6d:d9:0f:89:
        0c:b9:c7:68:1b:ba:52:22:23:61:7f:c0:5f:d6:80:94:74:01:
        cc:19:99:c5:49:11:83:a1:da:67:19:ce:c0:f7:64:b3:28:42:
        aa:bd:24:cf:a0:e5:9f:df:44:94:a9:f5:c5:82:68:b7:f2:68:
        d7:44:9a:8e:de:04:a0:83:2a:4d:14:17:c0:83:6c:eb:a5:87:
        68:99:f3:8f:1d:9d:fd:7f:9d:f9:8a:94:48:b9:68:de:f2:a7:
        30:41:50:82:d9:c0:fb:33:9b:63:b1:1d:5c:12:3c:3e:c9:82:
        6f:22:fa:89:bd:75:04:cf:7c:ca:d1:57:ae:7a:cc:c9:ff:50:
        00:cb:35:29:67:e3:cc:d2:b2:47:b3:b8:86:ed:9c:32:8c:0d:
        14:62:d6:bc:6d:4d:6e:f3:2d:d1:9e:d1:1f:3f:39:20:97:4d:
        2c:fd:51:1b:3e:70:cb:1e:16:bb:33:d5:58:fd:ca:93:26:8b:
        f9:ad:bb:38:9f:be:97:48:16:78:c8:6d:6e:a5:a7:3e:2a:11:
        81:fc:f3:c3:0c:b9:24:4a:f3:3c:d2:7a:54:2e:17:0b:41:52:
        d3:b5:46:9e
		*/

    char        buf[1024] = { 0 };
    const auto &s_name    = X509_get_subject_name(cert);
    const auto &str       = X509_NAME_oneline(s_name, buf, sizeof(buf));
    if (str)
    {
        std::cout << "subject:" << str << std::endl;
    }

    /// 发行
    const auto &issuer = X509_get_issuer_name(cert);
    const auto &str2   = X509_NAME_oneline(issuer, buf, sizeof(buf));
    if (str2)
    {
        std::cout << "issuer:" << str2 << std::endl;
    }
    X509_free(cert);
}

auto XSSL::write(const void *data, int data_size) -> int
{
    if (!impl_->ssl_)
    {
        return 0;
    }

    // int sock_fd = SSL_get_fd(impl_->ssl_);
    // return ::send(sock_fd, (char *)data, data_size, 0);

    return SSL_write(impl_->ssl_, data, data_size);
}

auto XSSL::read(void *buf, int buf_size) -> int
{
    if (!impl_->ssl_)
    {
        return 0;
    }

    // int sock_fd = SSL_get_fd(impl_->ssl_);
    // return ::recv(sock_fd, (char *)buf, buf_size, 0);

    return SSL_read(impl_->ssl_, buf, buf_size);
}

auto XSSL::close() -> void
{
    if (impl_->ssl_)
    {
        SSL_shutdown(impl_->ssl_);
        SSL_free(impl_->ssl_);
        impl_->ssl_ = nullptr;
    }
}
