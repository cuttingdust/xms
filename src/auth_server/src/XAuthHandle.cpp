#include "XAuthHandle.h"

#include "XAuthDao.h"
#include "XTools.h"

XAuthHandle::XAuthHandle() = default;

XAuthHandle::~XAuthHandle() = default;

void XAuthHandle::loginReq(xmsg::XMsgHead *head, XMsg *msg)
{
    int timeout_sec = 1800;

    ///用户登陆请求 MT_LOGIN_REQ
    xmsg::XLoginReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoginReq failed! ParseFromArray failed!");
        return;
    }
    /// 验证用户名密码
    xmsg::XLoginRes res;
    std::cout << req.DebugString();

    bool re = XAuthDao::get()->login(&req, &res, timeout_sec);
    if (!re)
    {
        LOGDEBUG("XAuthDao::get()->login failed!");
    }
    head->set_msgtype(xmsg::MT_LOGIN_RES);
    sendMsg(head, &res);
}

auto XAuthHandle::addUserReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    xmsg::XAddUserReq req;
    xmsg::XMessageRes res;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg("AddUserReq failed!ParseFromArray failed!");
        sendMsg(xmsg::MT_ADD_USER_RES, &res);
        return;
    }
    bool re = XAuthDao::get()->addUser(&req);
    if (!re)
    {
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg("XAuthDao::get()->AddUser failed!");
        sendMsg(xmsg::MT_ADD_USER_RES, &res);
        return;
    }
    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK");
    sendMsg(xmsg::MT_ADD_USER_RES, &res);
}

void XAuthHandle::regMsgCallback()
{
    regCB(xmsg::MT_LOGIN_REQ, static_cast<MsgCBFunc>(&XAuthHandle::loginReq));
    regCB(xmsg::MT_ADD_USER_REQ, static_cast<MsgCBFunc>(&XAuthHandle::addUserReq));
}
