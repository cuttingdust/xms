/**
 * @file   XService.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-25
 */

#ifndef XSERVICE_H
#define XSERVICE_H

#include "XPlatfrom_Global.h"
#include "XTask.h"
#include "XServiceHandle.h"

class XPLATFROM_EXPORT XService : public XTask
{
public:
    XService();
    virtual ~XService();

public:
    virtual XServiceHandle *createHandle() = 0;

    /// \brief �����������˿�
    /// \param port
    void setServerPort(int port);

    /// \brief �����ʼ�������̳߳ص���
    /// \return
    auto init() -> bool override;

    void listenCB(int client_socket, struct sockaddr *addr, int socketlen);

    /// \brief ��ʼ�������У� ��������������뵽�̳߳�
    /// \return
    bool start();

    void setSSLContent(XSSL_CTX *ctx);

    XSSL_CTX *getSSLContent() const;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};

#endif // XSERVICE_H
