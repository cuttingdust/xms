#include "XSSL_CTX.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>

static int SSLVerifyCB(int preverify_ok, X509_STORE_CTX *ctx)
{
    if (preverify_ok == 1)
    {
        std::cout << "SSLVerifyCB: preverify_ok = " << preverify_ok << std::endl;
        return 1;
    }
    else
    {
        std::cout << "SSLVerifyCB: preverify_ok = " << preverify_ok << std::endl;
        return 0;
    }
}


class XSSL_CTX::PImpl
{
public:
    PImpl(XSSL_CTX *owenr);
    ~PImpl() = default;

public:
    /// \brief 验证对方证书
    /// \param ca_crt
    void setVerify(const char *ca_crt);

public:
    XSSL_CTX *owenr_   = nullptr;
    SSL_CTX  *ssl_ctx_ = nullptr;
    SSL      *ssl_     = nullptr;
};

XSSL_CTX::PImpl::PImpl(XSSL_CTX *owenr) : owenr_(owenr)
{
    OpenSSL_add_ssl_algorithms();
    /*为打印调试信息作准备*/
    SSL_load_error_strings();
}

void XSSL_CTX::PImpl::setVerify(const char *ca_crt)
{
    if (!ca_crt || !ssl_ctx_)
        return;

    /// 设置验证对方证书
    SSL_CTX_set_verify(ssl_ctx_, SSL_VERIFY_PEER, SSLVerifyCB);
    SSL_CTX_load_verify_locations(ssl_ctx_, ca_crt, 0);
}

XSSL_CTX::XSSL_CTX()
{
    std::cout << "XSSL_CTX create!" << std::endl;
    impl_ = std::make_unique<XSSL_CTX::PImpl>(this);
}

XSSL_CTX::~XSSL_CTX()
{
    std::cout << "XSSL_CTX free!!!" << std::endl;
    if (impl_->ssl_)
    {
        SSL_free(impl_->ssl_);
        impl_->ssl_ = nullptr;
    }
};

auto XSSL_CTX::initServer(const char *crt_file, const char *key_file, const char *ca_file) -> bool
{
    /// 创建服务器 ssl ctx上下文
    impl_->ssl_ctx_ = SSL_CTX_new(TLS_server_method());
    if (!impl_->ssl_ctx_)
    {
        std::cerr << "SSL_CTX_new TLS_server_method failed!" << std::endl;
        return false;
    }

    /// 加载证书，私钥，并验证
    int re = SSL_CTX_use_certificate_file(impl_->ssl_ctx_, crt_file, SSL_FILETYPE_PEM);
    if (re <= 0)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    std::cout << "===========Load certificate success!===========" << std::endl;
    re = SSL_CTX_use_PrivateKey_file(impl_->ssl_ctx_, key_file, SSL_FILETYPE_PEM);
    if (re <= 0)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    std::cout << "===========Load PrivateKey success!=============" << std::endl;

    re = SSL_CTX_check_private_key(impl_->ssl_ctx_);
    if (re <= 0)
    {
        std::cout << "private key does not match the certificate!" << std::endl;
        return false;
    }
    std::cout << "===========check_private_key success!===========" << std::endl;

    /// 对服务器证书验证
    impl_->setVerify(ca_file);
    return true;
}

auto XSSL_CTX::initClient(const char *ca_file) -> bool
{
    impl_->ssl_ctx_ = SSL_CTX_new(TLS_client_method());
    if (!impl_->ssl_ctx_)
    {
        std::cerr << "SSL_CTX_new TLS_client_method failed!" << std::endl;
        return false;
    }
    /// 对客户端证书验证
    impl_->setVerify(ca_file);
    return true;
}

auto XSSL_CTX::createXSSL(int socket) -> XSSL::Ptr
{
    auto xssl = XSSL::create();
    if (!impl_->ssl_ctx_)
    {
        std::cout << "ssl_ctx == 0" << std::endl;
        return xssl;
    }

    impl_->ssl_ = SSL_new(impl_->ssl_ctx_);
    if (!impl_->ssl_)
    {
        std::cerr << "SSL_new failed!" << std::endl;
        return xssl;
    }
    /// bufferevent会自己创建
    if (socket > 0)
        SSL_set_fd(impl_->ssl_, socket);
    xssl->set_ssl(impl_->ssl_);

    return xssl;
}

auto XSSL_CTX::close() -> void
{
    if (impl_->ssl_ctx_)
    {
        SSL_CTX_free(impl_->ssl_ctx_);
        impl_->ssl_ctx_ = nullptr;
    }
}
