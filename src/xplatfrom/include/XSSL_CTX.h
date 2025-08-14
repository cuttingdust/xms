/**
 * @file   XSSL_CTX.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-04-21
 */

#ifndef XSSL_CTX_H
#define XSSL_CTX_H

#include "XSSL.h"
#include "XPlatfrom_Global.h"

#include <memory>

class XPLATFROM_EXPORT XSSL_CTX
{
public:
    XSSL_CTX();
    virtual ~XSSL_CTX();

public:
    /// \brief 初始化服务端
    /// \param crt_file 服务端证书文件
    /// \param key_file 服务端私钥文件
    /// \param ca_file 验证客户端证书（可选）
    /// \return 初始化是否成功
    virtual auto initServer(const char *crt_file, const char *key_file, const char *ca_file = 0) -> bool;

    /// \brief 初始化SSL客户端
    /// \param ca_file  验证服务端证书
    /// \return
    virtual auto initClient(const char *ca_file = 0) -> bool;

    /// \brief 创建SSL通信对象，socket和ssl_st资源由调用者释放
    /// 创建失败返回通过XSSL::isEmpty()判断
    /// \param socket
    /// \return
    auto createXSSL(int socket) -> XSSL::Ptr;

    /// \brief 释放资源
    auto close() -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XSSL_CTX_H
