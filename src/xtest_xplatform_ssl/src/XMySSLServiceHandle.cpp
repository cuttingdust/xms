#include "XMySSLServiceHandle.h"

XMySSLServiceHandle::XMySSLServiceHandle() = default;

XMySSLServiceHandle::~XMySSLServiceHandle() = default;

void XMySSLServiceHandle::regMsgCallback()
{
    regCB(xmsg::MT_LOGIN_REQ, static_cast<MsgCBFunc>(&XMySSLServiceHandle::loginReq));
}

void XMySSLServiceHandle::loginReq(xmsg::XMsgHead *head, XMsg *msg)
{
    std::cout << "LoginReq" << std::endl;
    xmsg::XLoginReq req;
    req.ParseFromArray(msg->data, msg->size);
    std::cout << req.DebugString() << std::endl;
}

void XMySSLServiceHandle::connectCB()
{
    std::cout << "service ssl accept" << std::endl;
}
