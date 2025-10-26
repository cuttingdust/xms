#include "XAuthHandle.h"

#include "XAuthDao.h"
#include "XTools.h"

XAuthHandle::XAuthHandle() = default;

XAuthHandle::~XAuthHandle() = default;

auto XAuthHandle::loginReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    int timeout_sec = 1800;

    ///用户登陆请求 MT_LOGIN_REQ
    xmsg::XLoginReq req;

    /// 登录返回消息
    xmsg::XLoginRes res;

    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoginReq failed!");
        res.set_restype(xmsg::XLoginRes::XRT_ERR);
        res.set_token("LoginReq ParseFromArray failed!");
        sendMsg(xmsg::MT_LOGIN_RES, &res);
        return;
    }
    /// 验证用户名密码
    std::cout << req.DebugString();

    bool re = XAuthDao::get()->login(&req, &res, timeout_sec);
    if (!re)
    {
        LOGDEBUG("XAuthDao::get()->login failed!");
        res.set_restype(xmsg::XLoginRes::XRT_ERR);
        res.set_token("username or password failed!");
    }
    head->set_msgtype(xmsg::MT_LOGIN_RES);
    sendMsg(head, &res);
}

auto XAuthHandle::checkTokenReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    xmsg::XLoginRes res;
    XAuthDao::get()->checkToken(head, &res);
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
    head->set_msgtype(xmsg::MT_ADD_USER_RES);
    sendMsg(head, &res);
}

auto XAuthHandle::changePasswordReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    xmsg::XChangePasswordReq req;
    xmsg::XMessageRes        res;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg("XChangePasswordReq ParseFromArray failed!");
        sendMsg(xmsg::MT_CHANGE_PASSWORD_RES, &res);
        return;
    }
    bool re = XAuthDao::get()->changePassword(&req);
    if (!re)
    {
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg("XAuthDao::Get()->ChangePassword failed!");
        sendMsg(xmsg::MT_CHANGE_PASSWORD_RES, &res);
        return;
    }
    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK!");

    head->set_msgtype(xmsg::MT_CHANGE_PASSWORD_RES);
    sendMsg(head, &res);
    //sendMsg(MT_CHANGE_PASSWORD_RES, &res);
}

void XAuthHandle::regMsgCallback()
{
    regCB(xmsg::MT_LOGIN_REQ, static_cast<MsgCBFunc>(&XAuthHandle::loginReq));
    regCB(xmsg::MT_ADD_USER_REQ, static_cast<MsgCBFunc>(&XAuthHandle::addUserReq));
    regCB(xmsg::MT_CHANGE_PASSWORD_REQ, static_cast<MsgCBFunc>(&XAuthHandle::changePasswordReq));
    regCB(xmsg::MT_CHECK_TOKEN_REQ, static_cast<MsgCBFunc>(&XAuthHandle::checkTokenReq));
}
