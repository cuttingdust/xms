/**
 * @file   XTestClient.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-25
 */

#ifndef XTESTCLIENT_H
#define XTESTCLIENT_H

#include "XServiceClient.h"

class XTestClient : public XServiceClient
{
public:
    void connectCB() override;

public:
    /// \brief
    /// \param path 请求的根目录
    /// \return 是否请求成功，不保证目录获取
    bool getDir(const std::string& path);

    /// \brief 目录请求相应
    /// \param head
    /// \param msg
    void dirRes(xmsg::XMsgHead* head, XMsg* msg);

    static void regMsgCallback();

    /// \brief 检查连接，自动重连,连接失败立刻返回，已连接立刻返回
    /// \param timeout_ms 超时时间
    /// \return 连接成功返回true
    bool autoConnect(int timeout_ms);
};

#endif // XTESTCLIENT_H
