#include "XRegisterClient.h"

#include <XTools.h>

#include <thread>
#include <fstream>

///ע������б�Ļ���
static xmsg::XServiceMap *service_map = nullptr;
static xmsg::XServiceMap *client_map  = nullptr;

/// ���̷߳��ʵ���
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
    /// ����ע����Ϣ
    LOGDEBUG("XRegisterClient::connectCB: connected start send MT_REGISTER_REQ ");
    xmsg::XRegisterReq req;
    req.set_name(impl_->service_name_);
    req.set_ip(impl_->service_ip_);
    req.set_port(impl_->service_port_);
    sendMsg(xmsg::MT_REGISTER_REQ, &req);
}

void XRegisterClient::registerServer(const char *service_name, int port, const char *ip)
{
    /// ע����Ϣ�ص�����
    regMsgCallback();
    /// ������Ϣ��������
    /// �����������Ƿ�ɹ���
    /// ע�����ĵ�IP��ע�����ĵĶ˿�
    if (service_name)
        strcpy(impl_->service_name_, service_name);
    if (ip)
        strcpy(impl_->service_ip_, ip);
    impl_->service_port_ = port;

    /// �����Զ�����
    setAutoConnect(true);

    /// ��������뵽�̳߳���
    startConnect();
}

void XRegisterClient::registerRes(xmsg::XMsgHead *head, XMsg *msg)
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
    LOGDEBUG(ss.str().c_str());
}

void XRegisterClient::getServiceReq(const char *service_name)
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

void XRegisterClient::getServiceRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("XRegisterClient::getServiceRes");
    XMutex mutex(&service_map_mutex);
    /// �Ƿ��滻ȫ������
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
    LOGDEBUG(cache_map->DebugString());

    if (cache_map->type() == xmsg::XT_ALL)
    {
        is_all = true;
    }

    ///////////////////////////////////////////////////////////
    /// �ڴ滺��ˢ��
    if (cache_map == service_map)
    {
        /// �洢�����Ѿ�ˢ��
    }
    else
    {
        if (is_all)
        {
            service_map->CopyFrom(*cache_map);
        }
        else
        {
            /// ���ն�ȡ��cmap���ݴ���  service_map �ڴ滺��
            auto cmap = cache_map->mutable_servicemap();

            /// ȡ��һ��
            if (!cmap || cmap->empty())
                return;
            auto one = cmap->begin();

            auto smap = service_map->mutable_servicemap();
            /// �޸Ļ���
            (*smap)[one->first] = one->second;
        }
    }


    ///////////////////////////////////////////////////////////
    /// ���̻���ˢ�� ����Ҫ����ˢ��Ƶ��
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
    /// �����ǻ�ȡһ�ֻ���ȫ�� ˢ�»���
    /// һ�� ֻˢ�´���΢�����б�������

    /// ȫ�� ˢ�����л�������
}

xmsg::XServiceMap *XRegisterClient::getAllService()
{
    XMutex mutex(&service_map_mutex);
    localLocalCache();
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
    /// 10ms�ж�һ��
    int totoal_count = timeout_sec * 100;
    int count        = 0;

    /// 1 �ȴ����ӳɹ�
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
        LOGDEBUG("���ӵȴ���ʱ");
        XMutex mutex(&service_map_mutex);
        /// ֻ�е�һ�ζ�ȡ����
        if (!service_map)
        {
            localLocalCache();
        }
        return result;
    }

    /// 2 ���ͻ�ȡ΢�������Ϣ
    getServiceReq(service_name);

    /// 3 �ȴ�΢�����б���Ϣ�������п����õ���һ�ε����ã�
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
            /// û���ҵ�ָ����΢����
            getServiceReq(service_name);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            count += 10;
            continue;
        }
        auto s = m->find(service_name);
        if (s == m->end())
        {
            // cout << "+" << flush;
            /// û���ҵ�ָ����΢����
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

auto XRegisterClient::localLocalCache() -> bool
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
        LOGDEBUG(log.str().c_str());
        return false;
    }
    service_map->ParseFromIstream(&ifs);
    ifs.close();
    return true;
}
