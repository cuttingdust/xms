/**
 * @file   XComTask.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-14
 */

#ifndef XCOMTASK_H
#define XCOMTASK_H

#include "XPlatfrom_Global.h"
#include "XTask.h"
#include "XMsg.h"

#include <memory>
#include <string>

class XSSL_CTX;

class XPLATFROM_EXPORT XComTask : public XTask
{
public:
    XComTask();
    ~XComTask() override;

public:
    /// \brief 初始化bufferevent，客户端建立连接
    /// \return
    auto init() -> bool override;

    /// \brief 开始连接服务器，调用成员 server_ip_ server_port_
    /// 考虑自动重连
    /// \return
    auto connect() const -> bool;

    /// \brief 是否已连接
    /// \return
    auto isConnected() const -> bool;

    /// \brief 是否正在连接
    /// \return
    auto isConnecting() const -> bool;

public:
    /// \brief 设置服务器ip
    /// \param ip
    auto setServerIP(const char* ip) -> void;
    auto getServerIP() const -> const char*;

    /// \brief 设置服务器端口
    /// \param port
    auto setServerPort(int port) -> void;
    auto getServerPort() const -> int;

    auto setServerRoot(const std::string path) -> void;

    /// \brief 设置客户端IP
    /// \param ip
    auto setClientIP(const char* ip) -> void;
    auto getClientIP() const -> char*;

    /// \brief 设置客户端端口
    /// \param port
    auto setClientPort(int port) -> void;
    auto getClientPort() const -> int;

    auto setLocalIP(const char* ip) -> void;
    auto getLocalIP() const -> char*;

    auto setLocalPort(int port) -> void;
    auto getLocalPort() const -> int;

    auto setIsRecvMsg(bool isRecvMsg) -> void;

    auto setAutoDelete(bool bAuto) -> void;

    /// \brief 是否自动重连 默认不自动 要在添加线程池之前设置
    /// 设置自动自动重连， 对象就不会自动删除
    /// \param bAuto
    auto setAutoConnect(bool bAuto) -> void;

    /// \brief 等待连接成功
    /// \param timeout_sec 最大等待时间
    auto waitConnected(int timeout_sec) -> bool;

    /// \brief 建立连接，如果断开，会再次重连，知道连接成功，或者超时
    /// \param timeout_sec
    /// \return
    auto autoConnect(int timeout_sec) -> bool;

    /// \brief 设置加密上下文
    /// \param ctx
    auto setSSLContent(XSSL_CTX* ctx) -> void;
    auto getSSLContent() const -> XSSL_CTX*;

    auto setTaskName(const char* name) -> void;
    auto taskName() const -> const char*;

    /// \brief 设定要在加入线程池之前
    /// \param ms
    auto setReadTimeMs(int ms) -> void;

    /// \brief 设定要在加入线程池之前 virtual void TimerCB() {}
    /// \param ms
    auto setTimeMs(int ms) -> void;

public:
    virtual auto eventCB(short events) -> void;

    virtual auto connectCB() -> void;

    virtual auto readCB() -> void = 0;

    virtual auto read(void* data, int size) -> int;

    virtual auto writeCB() -> void;

    virtual auto write(const void* data, int size) -> bool;

    /// \brief 激活写入回调
    virtual auto beginWriteCB() -> void;

    /// \brief 现有缓冲（未发送）的大小
    /// \return
    virtual auto bufferSize() -> long long;

    virtual auto close() -> void;

    /// \brief 清理所有定时器
    virtual auto clearTimer() -> void;

    virtual auto setTimer(int ms) -> void;

    virtual auto timerCB() -> void;

    /// \brief 设定自动重连的定时器
    /// \param ms
    virtual auto setAutoConnectTimer(int ms) -> void;

    /// \brief 自动重连定时器回调函数
    virtual auto autoConnectTimerCB() -> void;

public:
    /// \brief 连接服务端是否有错误
    /// \return
    auto hasError() const -> bool;

    /// \brief 连接错误原因
    /// \return
    auto error() const -> const char*;

    /// \brief 已经写入缓冲 （XMsg *msg ）的字节大小 不包含消息头
    /// \return
    auto getSendDataSize() const -> long long;

    /// \brief 已经读取缓冲 （XMsg *msg ）的字节大小 不包含消息头
    /// \return
    auto getRecvDataSize() const -> long long;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};

#endif // XCOMTASK_H
