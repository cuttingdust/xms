/**
 * @file   XSSL.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-04-21
 */

#ifndef XSSL_H
#define XSSL_H

#include <openssl/types.h>
#include <memory>

class XSSL
{
public:
    XSSL();
    virtual ~XSSL();

    using Ptr = std::shared_ptr<XSSL>;
    static XSSL::Ptr create()
    {
        return std::make_shared<XSSL>();
    }

    auto set_ssl(SSL *ssl) const -> void;
    auto get_ssl() const -> SSL *;

public:
    /// \brief 客户端处理ssl握手
    /// \return
    auto connect() -> bool;

    /// \brief
    /// \return
    auto isEmpty() const -> bool;

    /// \brief 服务端接收ssl连接
    /// \return
    auto accept() const -> bool;

    /// \brief 打印通信使用的算法
    auto printCipher() const -> void;

    /// \brief 打印对方证书信息
    auto printCert() const -> void;

    /// \brief 发送数据
    /// \param data
    /// \param data_size
    /// \return
    auto write(const void *data, int data_size) -> int;

    /// \brief 接收信息
    /// \param buf
    /// \param buf_size
    /// \return
    auto read(void *buf, int buf_size) -> int;

    /// \brief  释放资源
    auto close() -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XSSL_H
