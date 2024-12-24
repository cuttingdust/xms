#include "XConfigClient.h"

#include <XThreadPool.h>
#include <XTools.h>

/// key ip_port
static std::map<std::string, xmsg::XConfig> conf_map;
static std::mutex                           conf_map_mutex;

XConfigClient::XConfigClient()
{
}

XConfigClient::~XConfigClient()
{
}

void XConfigClient::sendConfig(xmsg::XConfig *conf)
{
    LOGDEBUG("��������");
    sendMsg(xmsg::MT_SAVE_CONFIG_REQ, conf);
}

void XConfigClient::sendConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("���յ��ϴ����õķ���");
    xmsg::XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("ParseFromArray failed!");
        return;
    }
    if (res.return_() == xmsg::XMessageRes::XR_OK)
    {
        LOGDEBUG("�ϴ����óɹ�!");
        return;
    }
    std::stringstream ss;
    ss << "�ϴ�����ʧ��:" << res.msg();
    LOGDEBUG(ss.str().c_str());
}

void XConfigClient::loadConfig(const char *ip, int port)
{
    LOGDEBUG("��ȡ��������");
    if (port < 0 || port > 65535)
    {
        LOGDEBUG("LoadConfig failed!port error");
        return;
    }
    xmsg::XLoadConfigReq req;
    if (ip) /// IP���ΪNULL ��ȡ�����������ĵĵ�ַ
        req.set_service_ip(ip);
    req.set_service_port(port);
    /// ������Ϣ�������
    sendMsg(xmsg::MT_LOAD_CONFIG_REQ, &req);
}

void XConfigClient::loadConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("��ȡ������Ӧ");
    xmsg::XConfig conf;
    if (!conf.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoadConfigRes conf.ParseFromArray failed!");
        return;
    }
    LOGDEBUG(conf.DebugString().c_str());

    /// key ip_port
    std::stringstream key;
    key << conf.service_ip() << "_" << conf.service_port();
    /// ��������
    conf_map_mutex.lock();
    conf_map[key.str()] = conf;
    conf_map_mutex.unlock();
}

bool XConfigClient::getConfig(const char *ip, int port, xmsg::XConfig *out_conf)
{
    std::stringstream key;
    key << ip << "_" << port;
    XMutex mutex(&conf_map_mutex);
    /// s��������
    auto conf = conf_map.find(key.str());
    if (conf == conf_map.end())
    {
        LOGDEBUG("Can`t find conf");
        return false;
    }
    /// ��������
    out_conf->CopyFrom(conf->second);
    return true;
}


void XConfigClient::regMsgCallback()
{
    regCB(xmsg::MT_SAVE_CONFIG_RES, static_cast<MsgCBFunc>(&XConfigClient::sendConfigRes));
    regCB(xmsg::MT_LOAD_CONFIG_RES, static_cast<MsgCBFunc>(&XConfigClient::loadConfigRes));
}

void XConfigClient::wait()
{
    XThreadPool::wait();
}
