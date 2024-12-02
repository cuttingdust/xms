#include "XRouteHandle.h"

#include "XServiceProxy.h"

#include <XTools.h>

XRouteHandle::XRouteHandle() = default;

XRouteHandle::~XRouteHandle() = default;

void XRouteHandle::readCB(xmsg::XMsgHead *head, XMsg *msg)
{
    /// 转发消息
    LOGDEBUG("XRouteHandle::readCB");
    XServiceProxy::get()->sendMsg(head, msg, this);
}

void XRouteHandle::close()
{
    LOGDEBUG("XRouteHandle::close");
    XServiceHandle::close();
    XServiceProxy::get()->delEvent(this);
}
