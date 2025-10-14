#include "XRouteHandle.h"

#include "XAuthProxy.h"
#include "XServiceProxy.h"

#include <XTools.h>

XRouteHandle::XRouteHandle() = default;

XRouteHandle::~XRouteHandle() = default;

void XRouteHandle::readCB(xmsg::XMsgHead *head, XMsg *msg)
{
    /// 转发消息
    LOGDEBUG("XRouteHandle::readCB");
    std::string token = head->token();
    std::string user  = head->username();
    if (head->msgtype() != xmsg::MT_LOGIN_REQ && !XAuthProxy::checkToken(head))
    {
        return;
    }

    XServiceProxy::get()->sendMsg(head, msg, this);
}

void XRouteHandle::close()
{
    LOGDEBUG("XRouteHandle::close");
    XServiceHandle::close();
    XServiceProxy::get()->delEvent(this);
}
