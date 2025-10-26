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
    static int i = 0;
    i++;
    std::cout << i << "XRouterHandle::ReadCB" << head->DebugString();
    std::string token = head->token();
    std::string user  = head->username();
    if (!head)
    {
        return;
    }

    if (head->msgtype() != xmsg::MT_LOGIN_REQ && !XAuthProxy::checkToken(head))
    {
        LOGINFO(head->DebugString());
        return;
    }
    head->set_msgid(reinterpret_cast<long long>(this));
    XServiceProxy::get()->sendMsg(head, msg, this);
}

void XRouteHandle::close()
{
    LOGDEBUG("XRouteHandle::close");
    XServiceHandle::close();
    XServiceProxy::get()->delEvent(this);
}

auto XRouteHandle::sendMsg(xmsg::XMsgHead *head, XMsg *msg) -> bool
{
    bool re = XMsgEvent::sendMsg(head, msg);
    if (re)
    {
        std::cout << "消息已回复" << head->DebugString() << std::endl;
    }
    else
    {
        std::cout << "消息回复异常" << head->DebugString() << std::endl;
    }
    return re;
}
