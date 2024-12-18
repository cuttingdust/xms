#include "XRegisterClient.h"

#include <XTools.h>

class XRegisterClient::PImpl
{
public:
    PImpl(XRegisterClient *owenr);
    ~PImpl() = default;

public:
    XRegisterClient *owenr_            = nullptr;
    char             service_name_[32] = { 0 };
    int              service_port_     = 0;
    char             service_ip_[16]   = { 0 };
};

XRegisterClient::PImpl::PImpl(XRegisterClient *owenr) : owenr_(owenr)
{
}

XRegisterClient::XRegisterClient()
{
    impl_ = std::make_unique<PImpl>(this);
}

XRegisterClient::~XRegisterClient() = default;

void XRegisterClient::connectCB()
{
    /// 发送注册消息
    LOGDEBUG("注册中心客户连接成功，开始发送注册请求！");
    xmsg::XRegisterReq req;
    req.set_name(impl_->service_name_);
    req.set_ip(impl_->service_ip_);
    req.set_port(impl_->service_port_);
    sendMsg(xmsg::MT_REGISTER_REQ, &req);
}

void XRegisterClient::registerServer(const char *service_name, int port, const char *ip)
{
    /// 注册消息回调函数
    regMsgCallback();
    /// 发送消息到服务器
    /// 服务器连接是否成功？
    /// 注册中心的IP，注册中心的端口
    if (service_name)
        strcpy(impl_->service_name_, service_name);
    if (ip)
        strcpy(impl_->service_ip_, ip);
    impl_->service_port_ = port;

    /// 把任务加入到线程池中
    startConnect();
}

void XRegisterClient::registerRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("接收到注册服务的响应");
    xmsg::XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XRegisterClient::RegisterRes failed!res.ParseFromArray failed!");
        return;
    }
    if (res.return_() == xmsg::XMessageRes::XR_OK)
    {
        LOGDEBUG("注册微服务成功");
        return;
    }
    std::stringstream ss;
    ss << "注册微服务失败 " << res.msg();
    LOGDEBUG(ss.str().c_str());
}

void XRegisterClient::getServiceReq(const char *service_name)
{
    LOGDEBUG("发出有获取微服务列表的请求");
    xmsg::XGetServiceReq req;
    if (service_name)
    {
        req.set_type(xmsg::XGetServiceReq::XT_ONE);
        req.set_name(service_name);
    }
    else
    {
        req.set_type(xmsg::XGetServiceReq::XT_ALL);
    }

    sendMsg(xmsg::MT_GET_SERVICE_REQ, &req);
}

void XRegisterClient::getServiceRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("获取服务列表的响应");
    xmsg::XServiceMap service_map;
    if (!service_map.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("service_map.ParseFromArray failed!");
        return;
    }
    LOGDEBUG(service_map.DebugString());
}

void XRegisterClient::regMsgCallback()
{
    regCB(xmsg::MT_REGISTER_RES, static_cast<MsgCBFunc>(&XRegisterClient::registerRes));
    regCB(xmsg::MT_GET_SERVICE_RES, static_cast<MsgCBFunc>(&XRegisterClient::getServiceRes));
}
