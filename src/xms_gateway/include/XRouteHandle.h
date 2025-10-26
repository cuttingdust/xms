/**
 * @file   XRouteHandle.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-11-28
 */

#ifndef XROUTEHANDLE_H
#define XROUTEHANDLE_H

#include <XServiceHandle.h>

class XRouteHandle : public XServiceHandle
{
public:
    XRouteHandle();
    ~XRouteHandle() override;

public:
    void readCB(xmsg::XMsgHead *head, XMsg *msg) override;

    /// \brief 连接断开，超时，出错调用
    void close() override;

    auto sendMsg(xmsg::XMsgHead *head, XMsg *msg) -> bool override;
};

#endif // XROUTEHANDLE_H
