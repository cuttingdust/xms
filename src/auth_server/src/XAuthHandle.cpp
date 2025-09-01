#include "XAuthHandle.h"

#include "XTools.h"

XAuthHandle::XAuthHandle() = default;

XAuthHandle::~XAuthHandle() = default;

void XAuthHandle::loginReq(xmsg::XMsgHead *head, XMsg *msg)
{
    ///用户登陆请求 MSG_LOGIN_REQ
    xmsg::XLoginReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoginReq failed! ParseFromArray failed!");
        return;
    }
    /// 验证用户名密码
    std::cout << req.DebugString();
}

void XAuthHandle::regMsgCallback()
{
    regCB(xmsg::MT_LOGIN_REQ, static_cast<MsgCBFunc>(&XAuthHandle::loginReq));
}
