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
    auto setServerPort(int port) -> void;

    /// \brief 服务初始化，由线程池调用
    /// \return
    auto init() -> bool override;

    auto listenCB(int client_socket, struct sockaddr *addr, int socketlen) -> void;

    /// \brief 开始服务运行， 接受连接任务加入到线程池
    /// \return
    auto start() -> bool;

    static auto wait() -> void;

    auto setSSLContent(XSSL_CTX *ctx) -> void;

    auto getSSLContent() const -> XSSL_CTX *;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};

#endif // XSERVICE_H
