#include "XRegisterClient.h"

#include <XTools.h>

#include <thread>

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
    LOGDEBUG("ע�����Ŀͻ����ӳɹ�����ʼ����ע������");
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
    LOGDEBUG("���յ�ע��������Ӧ");
    xmsg::XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XRegisterClient::RegisterRes failed!res.ParseFromArray failed!");
        return;
    }
    if (res.return_() == xmsg::XMessageRes::XR_OK)
    {
        LOGDEBUG("ע��΢����ɹ�");
        return;
    }
    std::stringstream ss;
    ss << "ע��΢����ʧ�� " << res.msg();
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
