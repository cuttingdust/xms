﻿#include "XServiceProxy.h"

#include "XServiceProxyClient.h"

#include <XTools.h>

#include <thread>

class XServiceProxy::PImpl
{
public:
    PImpl(XServiceProxy *owenr);
    ~PImpl();

public:
    void threadFunc();

public:
    XServiceProxy                                            *owenr_ = nullptr;
    std::map<std::string, std::vector<XServiceProxyClient *>> client_map_; ///< 存放与各个微服务的连接对象
    std::map<std::string, int>                                client_map_last_index_; ///< 记录上次轮询的索引
    bool                                                      is_exit_ = false;       ///< 是否退出
    std::map<XMsgEvent *, XServiceProxyClient *>              callbacks_;             ///< 用于清理callback缓冲
    std::mutex                                                callbacks_mutex_;
};

XServiceProxy::PImpl::PImpl(XServiceProxy *owenr) : owenr_(owenr)
{
}

XServiceProxy::PImpl::~PImpl() = default;

void XServiceProxy::PImpl::threadFunc()
{
    /// 自动重连
    while (!is_exit_)
    {
        for (const auto &[service_name, proxyClient] : client_map_)
        {
            for (const auto c : proxyClient)
            {
                if (c->isConnected())
                    continue;
                if (!c->isConnecting())
                {
                    LOGDEBUG("start conncet service ");
                    [[maybe_unused]] bool bRet = c->connect();
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void XServiceProxy::start()
{
    std::thread th(&PImpl::threadFunc, impl_.get());
    th.detach();
}

bool XServiceProxy::init()
{
    /// 1 从注册中心获取微服务列表
    xmsg::XServiceMap               service_map;
    auto                            map = service_map.mutable_servicemap();
    xmsg::XServiceMap::XServiceList list;
    {
        auto service = list.add_services();
        service->set_ip("127.0.0.1");
        service->set_port(20011);
        service->set_name("dir");
    }
    {
        auto service = list.add_services();
        service->set_ip("127.0.0.1");
        service->set_port(20012);
        service->set_name("dir");
    }
    {
        auto service = list.add_services();
        service->set_ip("127.0.0.1");
        service->set_port(20013);
        service->set_name("dir");
    }
    (*map)["dir"] = list;
    std::cout << service_map.DebugString() << std::endl;

    /// 2 与微服务建立连接
    /// 遍历XServiceMap数据
    for (const auto &[server_name, server_list] : (*map))
    {
        impl_->client_map_[server_name] = std::vector<XServiceProxyClient *>();
        for (const auto &s : server_list.services())
        {
            auto proxy = new XServiceProxyClient();
            proxy->setServerIp(s.ip().c_str());
            proxy->setServerPort(s.port());
            proxy->startConnect();
            impl_->client_map_[server_name].push_back(proxy);
            impl_->client_map_last_index_[server_name] = 0;
        }
    }

    return true;
}

/// 清理消息回调
void XServiceProxy::delEvent(XMsgEvent *ev)
{
    if (!ev)
        return;

    XMutex mux(&impl_->callbacks_mutex_);
    auto   call = impl_->callbacks_.find(ev);
    if (call == impl_->callbacks_.end())
    {
        LOGDEBUG("callbacks_ not find!");
    }
    call->second->delEvent(ev);
}

bool XServiceProxy::sendMsg(xmsg::XMsgHead *head, XMsg *msg, XMsgEvent *ev)
{
    if (!head || !msg)
        return false;

    auto service_name = head->servername();
    /// 1 负载均衡找到客户端连接，进行数据发送
    auto client_list = impl_->client_map_.find(service_name);
    if (client_list == impl_->client_map_.end())
    {
        std::stringstream ss;
        ss << service_name << "client_list not find!!! ";
        LOGDEBUG(ss.str().c_str());
        return false;
    }

    /// 2. 轮询找到可用的微服务连接
    int cur_index = impl_->client_map_last_index_[service_name];
    int list_size = client_list->second.size();
    for (int i = 0; i < list_size; i++)
    {
        cur_index++;
        cur_index %= list_size;
        impl_->client_map_last_index_[service_name] = cur_index;
        auto client                                 = client_list->second[cur_index];
        if (client->isConnected())
        {
            /// 用于退出清理
            XMutex mux(&impl_->callbacks_mutex_);
            impl_->callbacks_[ev] = client;

            /// 转发消息
            return client->sendMsg(head, msg, ev);
        }
    }

    LOGDEBUG("client not connected");
    return false;
}

XServiceProxy::XServiceProxy()
{
    impl_ = std::make_unique<PImpl>(this);
}

XServiceProxy::~XServiceProxy() = default;