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
    virtual auto createHandle() -> XServiceHandle * = 0;

    /// \brief 服务器监听端口
    /// \param port
    void setServerPort(int port);

    /// \brief 服务初始化，由线程池调用
    /// \return
    auto init() -> bool override;

    void listenCB(int client_socket, struct sockaddr *addr, int socketlen);

    /// \brief 开始服务运行， 接受连接任务加入到线程池
    /// \return
    bool start();

    void setSSLContent(XSSL_CTX *ctx);

    XSSL_CTX *getSSLContent() const;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};

#endif // XSERVICE_H
