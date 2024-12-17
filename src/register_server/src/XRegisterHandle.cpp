#include "XRegisterHandle.h"

#include <XTools.h>

void XRegisterHandle::registerReq(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("����˽��յ��û���ע������");

    /// ��Ӧ����Ϣ
    xmsg::XMessageRes res;

    ///��������
    xmsg::XRegisterReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XRegisterReq ParseFromArray failed!");
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg("XRegisterReq ParseFromArray failed!");
        sendMsg(xmsg::MT_REGISTER_RES, &res);
        return;
    }


    /// ���յ��û��ķ������ơ�����IP������˿�
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

    /// �����û�ע����Ϣ����
    std::stringstream ss;
    ss << "���յ��û�ע����Ϣ:" << service_name << "|" << service_ip << ":" << service_port;
    LOGINFO(ss.str().c_str());


    /// �洢�û�ע����Ϣ������Ѿ�ע����Ҫ����
    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK");
    sendMsg(xmsg::MT_REGISTER_RES, &res);
}

void XRegisterHandle::regMsgCallback()
{
    regCB(xmsg::MT_REGISTER_REQ, static_cast<MsgCBFunc>(&XRegisterHandle::registerReq));
}
