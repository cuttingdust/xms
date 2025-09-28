#include "XServiceProxy.h"

#include "XServiceProxyClient.h"

#include <XRegisterClient.h>
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
    std::map<std::string, std::vector<XServiceProxyClient *>> client_map_;            ///< 存放与各个微服务的连接对象
    std::map<std::string, int>                                client_map_last_index_; ///< 记录上次轮询的索引
    std::mutex                                                client_map_mutex_;
    bool                                                      is_exit_ = false; ///< 是否退出
    std::map<XMsgEvent *, XServiceProxyClient *>              callbacks_;       ///< 用于清理callback缓冲
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
        /// 从注册中心获取 微服务的列表更新
        /// 发送请求到注册中心
        XRegisterClient::get()->getServiceReq(NULL);
        auto service_map = XRegisterClient::get()->getAllService();
        if (!service_map)
        {
            LOGDEBUG("GetAllService : service_map is NULL");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        const auto &smap = service_map->servicemap();
        if (smap.empty())
        {
            LOGDEBUG("XServiceProxy : service_map->service_map is NULL");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        /// 遍历所有的微服务名称列表
        for (auto [service_name, service_map] : smap)
        {
            /// 遍历单个微服务
            for (auto s : service_map.services())
            {
                /// 不连接自己
                if (service_name == API_GATEWAY_NAME)
                {
                    continue;
                }

                /// 此微服务是否已经连接
                XMutex mux(&client_map_mutex_);
                /// 第一个微服务，创建对象，开启连接
                if (!client_map_.contains(service_name))
                {
                    client_map_[service_name] = std::vector<XServiceProxyClient *>(); ///< 创建对象
                }

                /// 列表中是否已有此微服务
                bool is_find = false;
                for (auto c : client_map_[service_name])
                {
                    if (s.ip() == c->getServerIP() && s.port() == c->getServerPort())
                    {
                        is_find = true;
                        break;
                    }
                }
                if (is_find)
                    continue;

                auto proxy = new XServiceProxyClient();
                proxy->setServerIp(s.ip().c_str());
                proxy->setServerPort(s.port());
                proxy->setAutoDelete(false);
                proxy->startConnect();
                client_map_[service_name].push_back(proxy);
                client_map_last_index_[service_name] = 0;
            }
        }

        /// 从注册中心获取 微服务的列表更新
        /// 定时全部重新获取
        for (const auto &[service_name, proxyClient] : client_map_)
        {
            for (const auto c : proxyClient)
            {
                if (c->isConnected())
                    continue;
                if (!c->isConnecting())
                {
                    LOGDEBUG("start conncet service ");
                    c->connect();
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
    // /// 1 从注册中心获取微服务列表
    // xmsg::XServiceMap               service_map;
    // auto                            map = service_map.mutable_servicemap();
    // xmsg::XServiceMap::XServiceList list;
    // {
    //     auto service = list.add_services();
    //     service->set_ip("127.0.0.1");
    //     service->set_port(20011);
    //     service->set_name("dir");
    // }
    // {
    //     auto service = list.add_services();
    //     service->set_ip("127.0.0.1");
    //     service->set_port(20012);
    //     service->set_name("dir");
    // }
    // {
    //     auto service = list.add_services();
    //     service->set_ip("127.0.0.1");
    //     service->set_port(20013);
    //     service->set_name("dir");
    // }
    // (*map)["dir"] = list;
    // std::cout << service_map.DebugString() << std::endl;
    //
    // /// 2 与微服务建立连接
    // /// 遍历XServiceMap数据
    // for (const auto &[server_name, server_list] : (*map))
    // {
    //     impl_->client_map_[server_name] = std::vector<XServiceProxyClient *>();
    //     for (const auto &s : server_list.services())
    //     {
    //         auto proxy = new XServiceProxyClient();
    //         proxy->setServerIp(s.ip().c_str());
    //         proxy->setServerPort(s.port());
    //         proxy->startConnect();
    //         impl_->client_map_[server_name].push_back(proxy);
    //         impl_->client_map_last_index_[server_name] = 0;
    //     }
    // }

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
        return;
    }
    call->second->delEvent(ev);
}

bool XServiceProxy::sendMsg(xmsg::XMsgHead *head, XMsg *msg, XMsgEvent *ev)
{
    if (!head || !msg)
        return false;

    auto service_name = head->servername();

    XMutex mux(&impl_->client_map_mutex_);
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
