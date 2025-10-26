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

    /// \brief 检查token
    /// \param head
    /// \param msg
    auto checkTokenReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    /// \brief 接收添加用户消息
    /// \param head
    /// \param msg
    auto addUserReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    /// \brief 接收修改密码消息
    /// \param head
    /// \param msg
    auto changePasswordReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    static void regMsgCallback();
};


#endif // XAUTHHANDLE_H
