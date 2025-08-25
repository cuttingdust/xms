/**
 * @file   XRegisterHandle.h
 * @brief  处理注册中心的客户端 对应一个连接
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-17
 */

#ifndef XREGISTERHANDLE_H
#define XREGISTERHANDLE_H

#include "XServiceHandle.h"

class XRegisterHandle : public XServiceHandle
{
public:
    /// \brief 接收服务的注册请求
    /// \param head
    /// \param msg
    auto registerReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    /// \brief 接收服务的发现请求
    /// \param head
    /// \param msg
    auto getServiceReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto heartRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    static auto regMsgCallback() -> void;
};


#endif // XREGISTERHANDLE_H
