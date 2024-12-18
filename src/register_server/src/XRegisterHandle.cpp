#include "XRegisterHandle.h"

#include <XTools.h>

/// ע������б�Ļ���
static xmsg::XServiceMap *service_map = 0;

/// ���̷߳��ʵ���
static std::mutex service_map_mutex;

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
    {
        XMutex mutex(&service_map_mutex);
        if (!service_map)
            service_map = new xmsg::XServiceMap();
        auto smap = service_map->mutable_servicemap();

        /// �Ƿ���ͬ�����Ѿ�ע��
        /// ��Ⱥ΢����
        auto service_list = smap->find(service_name);
        if (service_list == smap->end())
        {
            /// û��ע���
            (*smap)[service_name] = xmsg::XServiceMap::XServiceList();
            service_list          = smap->find(service_name);
        }
        auto services = service_list->second.mutable_services();
        /// �����Ƿ���ͬip�Ͷ˿ڵ�
        for (const auto &service : (*services))
        {
            if (service.ip() == service_ip && service.port() == service_port)
            {
                std::stringstream ss;
                ss << service_name << "|" << service_ip << ":" << service_port << "΢�����Ѿ�ע���";
                LOGDEBUG(ss.str().c_str());
                res.set_return_(xmsg::XMessageRes::XR_ERROR);
                res.set_msg(ss.str());
                sendMsg(xmsg::MT_REGISTER_RES, &res);
                return;
            }
        }
        /// ����µ�΢����
        auto ser = service_list->second.add_services();
        ser->set_ip(service_ip);
        ser->set_port(service_port);
        ser->set_name(service_name);
        std::stringstream ss;
        ss << service_name << "|" << service_ip << ":" << service_port << "�µ�΢����ע��ɹ���";
        LOGDEBUG(ss.str().c_str());
    }

    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK");
    sendMsg(xmsg::MT_REGISTER_RES, &res);
}

void XRegisterHandle::getServiceReq(xmsg::XMsgHead *head, XMsg *msg)
{
    /// ��ʱֻ����ȫ��
    LOGDEBUG("���շ���ķ�������");
    xmsg::XGetServiceReq req;

    /// ������
    xmsg::XServiceMap res;
    res.mutable_res()->set_return_(xmsg::XMessageRes_XReturn::XMessageRes_XReturn_XR_ERROR);
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        std::stringstream ss;
        ss << "req.ParseFromArray failed!";
        LOGDEBUG(ss.str().c_str());
        res.mutable_res()->set_msg(ss.str().c_str());
        sendMsg(xmsg::MT_GET_SERVICE_RES, &res);
        return;
    }
    std::string       service_name = req.name();
    std::stringstream ss;
    ss << "GetServiceReq : service name " << service_name;
    LOGDEBUG(ss.str().c_str());

    /// ����ȫ��΢��������
    service_map_mutex.lock();
    service_map->mutable_res()->set_return_(xmsg::XMessageRes_XReturn::XMessageRes_XReturn_XR_OK);
    sendMsg(xmsg::MT_GET_SERVICE_RES, service_map);
    service_map_mutex.unlock();
}

void XRegisterHandle::regMsgCallback()
{
    regCB(xmsg::MT_REGISTER_REQ, static_cast<MsgCBFunc>(&XRegisterHandle::registerReq));
    regCB(xmsg::MT_GET_SERVICE_REQ, static_cast<MsgCBFunc>(&XRegisterHandle::getServiceReq));
}
