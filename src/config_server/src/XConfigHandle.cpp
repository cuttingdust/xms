#include "XConfigHandle.h"

#include "ConfigDao.h"

#include <XTools.h>

XConfigHandle::XConfigHandle()
{
}

XConfigHandle::~XConfigHandle()
{
}

void XConfigHandle::saveConfig(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("接收到保存配置的消息");
    xmsg::XMessageRes res;
    xmsg::XConfig     conf;
    if (!conf.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XConfigHandle::SaveConfig failed! format error!");
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg(" format erro");
        sendMsg(xmsg::MT_SAVE_CONFIG_RES, &res);
        return;
    }
    if (ConfigDao::get()->saveConfig(&conf))
    {
        res.set_return_(xmsg::XMessageRes::XR_OK);
        res.set_msg("OK");
        sendMsg(xmsg::MT_SAVE_CONFIG_RES, &res);
        return;
    }
    res.set_return_(xmsg::XMessageRes::XR_ERROR);
    res.set_msg("insert db failed!");
    sendMsg(xmsg::MT_SAVE_CONFIG_RES, &res);
}

void XConfigHandle::loadConfig(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("Received message to download configuration...");
    xmsg::XLoadConfigReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoadConfig ParseFromArray failed!");
        return;
    }

    std::string ip = req.service_ip();
    if (ip.empty())
    {
        ip = clientIP();
    }

    /// 根据IP和端口获取配置项
    xmsg::XConfig conf = ConfigDao::get()->loadConfig(ip.c_str(), req.service_port());

    /// 发送给客户端
    sendMsg(xmsg::MT_LOAD_CONFIG_RES, &conf);
}

void XConfigHandle::regMsgCallback()
{
    regCB(xmsg::MT_SAVE_CONFIG_REQ, static_cast<MsgCBFunc>(&XConfigHandle::saveConfig));
    regCB(xmsg::MT_LOAD_CONFIG_REQ, static_cast<MsgCBFunc>(&XConfigHandle::loadConfig));
    regCB(xmsg::MT_LOAD_ALL_CONFIG_REQ, static_cast<MsgCBFunc>(&XConfigHandle::loadAllConfig));
    regCB(xmsg::MT_DEL_CONFIG_REQ, static_cast<MsgCBFunc>(&XConfigHandle::deleteConfig));
}

void XConfigHandle::loadAllConfig(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("下载全部配置（有分页）");
    xmsg::XLoadAllConfigReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoadAllConfig ParseFromArray failed!");
        return;
    }
    const auto config_list = ConfigDao::get()->loadAllConfig(req.page(), req.page_count());
    /// 发送给客户端
    sendMsg(xmsg::MT_LOAD_ALL_CONFIG_RES, &config_list);
}

void XConfigHandle::deleteConfig(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("接收到删除配置的消息");
    xmsg::XMessageRes    res;
    xmsg::XLoadConfigReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("DeleteConfig ParseFromArray failed!");
        return;
    }
    if (ConfigDao::get()->deleteConfig(req.service_ip().c_str(), req.service_port()))
    {
        res.set_return_(xmsg::XMessageRes::XR_OK);
        res.set_msg("OK");
        sendMsg(xmsg::MT_DEL_CONFIG_RES, &res);
        return;
    }
    res.set_return_(xmsg::XMessageRes::XR_ERROR);
    res.set_msg("delete db failed!");
    sendMsg(xmsg::MT_DEL_CONFIG_RES, &res);
}
