#include "XRegisterHandle.h"

#include <XTools.h>

void XRegisterHandle::registerReq(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("服务端接收到用户的注册请求");

    /// 回应的消息
    xmsg::XMessageRes res;

    ///解析请求
    xmsg::XRegisterReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XRegisterReq ParseFromArray failed!");
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg("XRegisterReq ParseFromArray failed!");
        sendMsg(xmsg::MT_REGISTER_RES, &res);
        return;
    }


    /// 接收到用户的服务名称、服务IP、服务端口
    std::string service_name = req.name();
    if (service_name.empty())
    {
        std::string error = "service_name is empty!";
        LOGDEBUG(error.c_str());
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg(error);
        sendMsg(xmsg::MT_REGISTER_RES, &res);
        return;
    }
    std::string service_ip = req.ip();
    if (service_ip.empty())
    {
        LOGDEBUG("service_ip is empty : client ip");
        service_ip = this->clientIP();
    }

    int service_port = req.port();
    if (service_port <= 0 || service_port > 65535)
    {
        std::stringstream ss;
        //string error = "service_port is error!";
        ss << "service_port is error!" << service_port;
        LOGDEBUG(ss.str().c_str());
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg(ss.str());
        sendMsg(xmsg::MT_REGISTER_RES, &res);
        return;
    }

    /// 接收用户注册信息正常
    std::stringstream ss;
    ss << "接收到用户注册信息:" << service_name << "|" << service_ip << ":" << service_port;
    LOGINFO(ss.str().c_str());


    /// 存储用户注册信息，如果已经注册需要更新
    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK");
    sendMsg(xmsg::MT_REGISTER_RES, &res);
}

void XRegisterHandle::regMsgCallback()
{
    regCB(xmsg::MT_REGISTER_REQ, static_cast<MsgCBFunc>(&XRegisterHandle::registerReq));
}
