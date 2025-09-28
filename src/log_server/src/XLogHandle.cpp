#include "XLogHandle.h"

#include "XLogDao.h"

XLogHandle::XLogHandle()
{
}

XLogHandle::~XLogHandle() = default;

auto XLogHandle::addLogReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    xmsg::XAddLogReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        std::cerr << "AddLogReq failed! ParseFromArray" << std::endl;
        return;
    }
    if (req.service_ip().empty())
    {
        req.set_service_ip(getClientIP());
    }

    XLogDao::get()->addLog(&req);
}

auto XLogHandle::regMsgCallback() -> void
{
    regCB(xmsg::MT_ADD_LOG_REQ, static_cast<MsgCBFunc>(&XLogHandle::addLogReq));
}
