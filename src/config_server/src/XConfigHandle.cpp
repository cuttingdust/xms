﻿#include "XConfigHandle.h"

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
    LOGDEBUG("接收到下载配置的消息");
    xmsg::XLoadConfigReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoadConfig ParseFromArray failed!");
        return;
    }
    /// 根据IP和端口获取配置项
    xmsg::XConfig conf = ConfigDao::get()->loadConfig(req.service_ip().c_str(), req.service_port());

    /// 发送给客户端
    sendMsg(xmsg::MT_LOAD_CONFIG_RES, &conf);
}

void XConfigHandle::regMsgCallback()
{
    regCB(xmsg::MT_SAVE_CONFIG_REQ, static_cast<MsgCBFunc>(&XConfigHandle::saveConfig));
    regCB(xmsg::MT_LOAD_CONFIG_REQ, static_cast<MsgCBFunc>(&XConfigHandle::loadConfig));
}