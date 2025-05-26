#include "XRegisterClient.h"

#include <XTools.h>

#include <thread>

///注册服务列表的缓存
static xmsg::XServiceMap *service_map = nullptr;
static xmsg::XServiceMap *client_map  = nullptr;

/// 多线程访问的锁
static std::mutex service_map_mutex;

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

    /// 设置自动重连
    setAutoConnect(true);

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
    LOGDEBUG("getServiceReq");
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
    LOGDEBUG("getServiceRes");
    XMutex mutex(&service_map_mutex);
    if (!service_map)
    {
        service_map = new xmsg::XServiceMap;
    }

    if (!service_map->ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("service_map.ParseFromArray failed!");
        return;
    }
    LOGDEBUG(service_map->DebugString());
}

xmsg::XServiceMap *XRegisterClient::getAllService() const
{
    XMutex mutex(&service_map_mutex);
    if (!service_map)
    {
        return nullptr;
    }
    if (!client_map)
    {
        client_map = new xmsg::XServiceMap();
    }
    client_map->CopyFrom(*service_map);
    return client_map;
}

auto XRegisterClient::getServices(const char *service_name, int timeout_sec) -> xmsg::XServiceMap::XServiceList
{
    xmsg::XServiceMap::XServiceList result;
    /// 10ms判断一次
    int totoal_count = timeout_sec * 100;
    int count        = 0;

    /// 1 等待连接成功
    while (count < totoal_count)
    {
        //cout << "@" << flush;
        if (isConnected())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        count++;
    }

    if (!isConnected())
    {
        LOGDEBUG("连接等待超时");
        return result;
    }

    /// 2 发送获取微服务的消息
    getServiceReq(service_name);

    /// 3 等待微服务列表消息反馈（有可能拿到上一次的配置）
    while (count < totoal_count)
    {
        std::cout << "." << std::flush;
        XMutex mutex(&service_map_mutex);
        if (!service_map)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            count++;
            continue;
        }
        auto m = service_map->mutable_servicemap();
        if (!m)
        {
            //cout << "#" << flush;
            /// 没有找到指定的微服务
            getServiceReq(service_name);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            count += 10;
            continue;
        }
        auto s = m->find(service_name);
        if (s == m->end())
        {
            // cout << "+" << flush;
            /// 没有找到指定的微服务
            getServiceReq(service_name);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            count += 10;
            continue;
        }
        result.CopyFrom(s->second);
        return result;
    }
    return result;
}

void XRegisterClient::regMsgCallback()
{
    regCB(xmsg::MT_REGISTER_RES, static_cast<MsgCBFunc>(&XRegisterClient::registerRes));
    regCB(xmsg::MT_GET_SERVICE_RES, static_cast<MsgCBFunc>(&XRegisterClient::getServiceRes));
}
