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
    /// \brief �ͻ��˴���ssl����
    /// \return
    auto connect() -> bool;

    /// \brief
    /// \return
    auto isEmpty() const -> bool;

    /// \brief ����˽���ssl����
    /// \return
    auto accept() const -> bool;

    /// \brief ��ӡͨ��ʹ�õ��㷨
    auto printCipher() const -> void;

    /// \brief ��ӡ�Է�֤����Ϣ
    auto printCert() const -> void;

    /// \brief ��������
    /// \param data
    /// \param data_size
    /// \return
    auto write(const void *data, int data_size) -> int;

    /// \brief ������Ϣ
    /// \param buf
    /// \param buf_size
    /// \return
    auto read(void *buf, int buf_size) -> int;

    /// \brief  �ͷ���Դ
    auto close() -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XSSL_H
