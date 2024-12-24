#include "XConfigClient.h"

#include <XThreadPool.h>
#include <XTools.h>

XConfigClient::XConfigClient()
{
}

XConfigClient::~XConfigClient()
{
}

void XConfigClient::sendConfig(xmsg::XConfig *conf)
{
    LOGDEBUG("发送配置");
    sendMsg(xmsg::MT_SAVE_CONFIG_REQ, conf);
}

void XConfigClient::sendConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("接收到上传配置的反馈");
    xmsg::XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("ParseFromArray failed!");
        return;
    }
    if (res.return_() == xmsg::XMessageRes::XR_OK)
    {
        LOGDEBUG("上传配置成功!");
        return;
    }
    std::stringstream ss;
    ss << "上传配置失败:" << res.msg();
    LOGDEBUG(ss.str().c_str());
}

void XConfigClient::loadConfig(const char *ip, int port)
{
    LOGDEBUG("获取配置请求");
    if (port < 0 || port > 65535)
    {
        LOGDEBUG("LoadConfig failed!port error");
        return;
    }
    xmsg::XLoadConfigReq req;
    if (ip) /// IP如果为NULL 则取连接配置中心的地址
        req.set_service_ip(ip);
    req.set_service_port(port);
    /// 发送消息到服务端
    sendMsg(xmsg::MT_LOAD_CONFIG_REQ, &req);
}

void XConfigClient::loadConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("获取配置响应");
    xmsg::XConfig conf;
    if (!conf.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoadConfigRes conf.ParseFromArray failed!");
        return;
    }
    LOGDEBUG(conf.DebugString().c_str());
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
