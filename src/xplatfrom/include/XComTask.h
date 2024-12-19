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

class XPLATFROM_EXPORT XComTask : public XTask
{
public:
    XComTask();
    virtual ~XComTask();

public:
    /// \brief 初始化bufferevent，客户端建立连接
    /// \return
    auto init() -> bool override;

    /// \brief 开始连接服务器，调用成员 server_ip_ server_port_
    /// 考虑自动重连
    /// \return
    auto connect() const -> bool;

    // protected:
    /// \brief 是否已连接
    /// \return
    auto isConnected() const -> bool;

    /// \brief 是否正在连接
    /// \return
    auto isConnecting() const -> bool;

public:
    /// \brief 设置服务器ip
    /// \param ip
    void        setServerIp(const char* ip);
    const char* getServerIp() const;

    /// \brief 设置服务器端口
    /// \param port
    void setServerPort(int port);
    int  getServerPort() const;

    void setServerRoot(const std::string path);

    void setIsRecvMsg(bool isRecvMsg);

    void setAutoDelete(bool bAuto);

    /// \brief 等待连接成功
    /// \param timeout_sec 最大等待时间
    bool waitConnected(int timeout_sec);

public:
    virtual void eventCB(short events);

    virtual void connectCB();

    virtual void readCB() = 0;

    virtual int read(void* data, int size);

    virtual void writeCB();

    virtual bool write(const void* data, int size);

    /// \brief 激活写入回调
    virtual void beginWriteCB();

    virtual void close();

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};

#endif // XCOMTASK_H
