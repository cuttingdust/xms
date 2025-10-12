#include "XRegisterClient.h"

#include <XTools.h>

#include <thread>
#include <fstream>

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

auto XRegisterClient::connectCB() -> void
{
    /// 发送注册消息
    LOGDEBUG("XRegisterClient::connectCB: connected start send MT_REGISTER_REQ ");
    xmsg::XRegisterReq req;
    req.set_name(impl_->service_name_);
    req.set_ip(impl_->service_ip_);
    req.set_port(impl_->service_port_);
    sendMsg(xmsg::MT_REGISTER_REQ, &req);
}

auto XRegisterClient::timerCB() -> void
{
    /// 定时器，用于发送心跳
    static long long count = 0;
    count++;
    xmsg::XMsgHeart req;
    req.set_count(count);
    sendMsg(xmsg::MT_HEART_REQ, &req);
}

auto XRegisterClient::registerServer(const char *service_name, int port, const char *ip) -> void
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

    /// 设定心跳定时器
    setTimeMs(3000);

    /// 把任务加入到线程池中
    startConnect();

    loadLocalCache();
}

auto XRegisterClient::registerRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    LOGDEBUG("XRegisterClient::registerRes");
    xmsg::XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XRegisterClient::RegisterRes failed!res.ParseFromArray failed!");
        return;
    }
    if (res.return_() == xmsg::XMessageRes::XR_OK)
    {
        LOGDEBUG("XRegisterClient::registerRes success");
        return;
    }
    std::stringstream ss;
    ss << "XRegisterClient::registerRes failed!!! " << res.msg();
    LOGDEBUG(ss.str());
}

auto XRegisterClient::getServiceReq(const char *service_name) -> void
{
    LOGDEBUG("XRegisterClient::getServiceReq");
    xmsg::XGetServiceReq req;
    if (service_name)
    {
        req.set_type(xmsg::XT_ONE);
        req.set_name(service_name);
    }
    else
    {
        req.set_type(xmsg::XT_ALL);
    }

    sendMsg(xmsg::MT_GET_SERVICE_REQ, &req);
}

auto XRegisterClient::getServiceRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    LOGDEBUG("XRegisterClient::getServiceRes");
    XMutex mutex(&service_map_mutex);
    /// 是否替换全部缓存
    bool               is_all = false;
    xmsg::XServiceMap *cache_map;
    xmsg::XServiceMap  tmp;
    cache_map = &tmp;
    if (!service_map)
    {
        service_map = new xmsg::XServiceMap();
        cache_map   = service_map;
        is_all      = true;
    }

    if (!cache_map->ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("service_map.ParseFromArray failed!");
        return;
    }
    if (cache_map->type() == xmsg::XT_ALL)
    {
        is_all = true;
    }

    ///////////////////////////////////////////////////////////
    /// 内存缓存刷新
    if (cache_map == service_map)
    {
        /// 存储缓存已经刷新
    }
    else
    {
        if (is_all)
        {
            service_map->CopyFrom(*cache_map);
        }
        else
        {
            /// 将刚读取的cmap数据存入  service_map 内存缓冲
            auto cmap = cache_map->mutable_servicemap();

            /// 取第一个
            if (!cmap || cmap->empty())
                return;
            auto one = cmap->begin();

            auto smap = service_map->mutable_servicemap();
            /// 修改缓存
            (*smap)[one->first] = one->second;
        }
    }


    ///////////////////////////////////////////////////////////
    /// 磁盘缓存刷新 后期要考虑刷新频率
    std::stringstream ss;
    ss << "register_" << impl_->service_name_ << impl_->service_ip_ << impl_->service_port_ << ".cache";
    LOGDEBUG("Save local file!");
    if (!service_map)
        return;
    std::ofstream ofs;
    ofs.open(ss.str(), std::ios::binary);
    if (!ofs.is_open())
    {
        LOGDEBUG("save local file failed!");
        return;
    }
    service_map->SerializePartialToOstream(&ofs);
    ofs.close();

    /// LOGDEBUG(service_map->DebugString());
    /// 区分是获取一种还是全部 刷新缓存
    /// 一种 只刷新此种微服务列表缓存数据

    /// 全部 刷新所有缓存数据
}

auto XRegisterClient::getAllService() -> xmsg::XServiceMap *
{
    XMutex mutex(&service_map_mutex);
    // loadLocalCache();
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
    int total_count = timeout_sec * 100;
    int count       = 0;

    /// 1 等待连接成功
    while (count < total_count)
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
        XMutex mutex(&service_map_mutex);
        /// 只有第一次读取缓存
        if (!service_map)
        {
            loadLocalCache();
        }
        return result;
    }

    /// 2 发送获取微服务的消息
    getServiceReq(service_name);

    /// 3 等待微服务列表消息反馈（有可能拿到上一次的配置）
    while (count < total_count)
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

auto XRegisterClient::regMsgCallback() -> void
{
    regCB(xmsg::MT_REGISTER_RES, static_cast<MsgCBFunc>(&XRegisterClient::registerRes));
    regCB(xmsg::MT_GET_SERVICE_RES, static_cast<MsgCBFunc>(&XRegisterClient::getServiceRes));
}

auto XRegisterClient::loadLocalCache() -> bool
{
    if (!service_map)
    {
        service_map = new xmsg::XServiceMap();
    }
    LOGDEBUG("Load local register data");
    std::stringstream ss;
    ss << "register_" << impl_->service_name_ << impl_->service_ip_ << impl_->service_port_ << ".cache";
    std::ifstream ifs;
    ifs.open(ss.str(), std::ios::binary);
    if (!ifs.is_open())
    {
        std::stringstream log;
        log << "Load local register data failed!";
        log << ss.str();
        LOGDEBUG(log.str());
        return false;
    }
    service_map->ParseFromIstream(&ifs);
    ifs.close();
    return true;
}
