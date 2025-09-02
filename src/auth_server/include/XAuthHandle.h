/**
 * @file   XAuthHandle.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-09-01
 */

#ifndef XAUTHHANDLE_H
#define XAUTHHANDLE_H

#include <XServiceHandle.h>

class XAuthHandle : public XServiceHandle
{
public:
    XAuthHandle();
    ~XAuthHandle() override;

public:
    /// \brief 接收登录请求
    /// \param head
    /// \param msg
    auto loginReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto addUserReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    static void regMsgCallback();
};


#endif // XAUTHHANDLE_H
