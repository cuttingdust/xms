#include "XConfigClient.h"

#include <XThreadPool.h>
#include <XTools.h>

/// key ip_port
static std::map<std::string, xmsg::XConfig> conf_map;
static std::mutex                           conf_map_mutex;

/// �洢��ǰ΢��������
static google::protobuf::Message *cur_service_conf = nullptr;
static std::mutex                 cur_service_conf_mutex;

class XConfigClient::PImpl
{
public:
    PImpl(XConfigClient *owenr);
    ~PImpl() = default;

public:
    XConfigClient *owenr_ = nullptr;
    /// ����΢�����ip�Ͷ˿�
    char local_ip_[16] = { 0 };
    int  local_port_   = 0;
};

XConfigClient::PImpl::PImpl(XConfigClient *owenr) : owenr_(owenr)
{
}


XConfigClient::XConfigClient()
{
    impl_ = std::make_unique<PImpl>(this);
}

XConfigClient::~XConfigClient()
{
}

void XConfigClient::sendConfig(xmsg::XConfig *conf)
{
    LOGDEBUG("��������");
    sendMsg(xmsg::MT_SAVE_CONFIG_REQ, conf);
}

void XConfigClient::sendConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("���յ��ϴ����õķ���");
    xmsg::XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("ParseFromArray failed!");
        return;
    }
    if (res.return_() == xmsg::XMessageRes::XR_OK)
    {
        LOGDEBUG("�ϴ����óɹ�!");
        return;
    }
    std::stringstream ss;
    ss << "�ϴ�����ʧ��:" << res.msg();
    LOGDEBUG(ss.str().c_str());
}

void XConfigClient::loadConfig(const char *ip, int port)
{
    LOGDEBUG("��ȡ��������");
    if (port < 0 || port > 65535)
    {
        LOGDEBUG("LoadConfig failed!port error");
        return;
    }
    xmsg::XLoadConfigReq req;
    if (ip) /// IP���ΪNULL ��ȡ�����������ĵĵ�ַ
        req.set_service_ip(ip);
    req.set_service_port(port);
    /// ������Ϣ�������
    sendMsg(xmsg::MT_LOAD_CONFIG_REQ, &req);
}

void XConfigClient::loadConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("��ȡ������Ӧ");
    xmsg::XConfig conf;
    if (!conf.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("LoadConfigRes conf.ParseFromArray failed!");
        return;
    }
    LOGDEBUG(conf.DebugString().c_str());

    /// key ip_port
    std::stringstream key;
    key << conf.service_ip() << "_" << conf.service_port();
    /// ��������
    conf_map_mutex.lock();
    conf_map[key.str()] = conf;
    conf_map_mutex.unlock();

    /// �洢��������
    if (impl_->local_port_ > 0 && cur_service_conf)
    {
        std::stringstream local_key;
        local_key << impl_->local_ip_ << "_" << impl_->local_port_;
        if (key.str() == local_key.str())
        {
            std::cout << "%" << std::flush;
            //cout << "%"<< conf.DebugString() << flush;
            XMutex mux(&cur_service_conf_mutex);
            if (cur_service_conf)
                cur_service_conf->ParseFromString(conf.private_pb());
        }
    }
}

bool XConfigClient::getConfig(const char *ip, int port, xmsg::XConfig *out_conf)
{
    std::stringstream key;
    key << ip << "_" << port;
    XMutex mutex(&conf_map_mutex);
    /// s��������
    auto conf = conf_map.find(key.str());
    if (conf == conf_map.end())
    {
        LOGDEBUG("Can`t find conf");
        return false;
    }
    /// ��������
    out_conf->CopyFrom(conf->second);
    return true;
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

bool XConfigClient::startGetConf(const char *server_ip, int server_port, const char *local_ip, int local_port,
                                 google::protobuf::Message *conf_message, int timeout_sec)
{
    regMsgCallback();
    setServerIp(server_ip);
    setServerPort(server_port);
    strncpy(impl_->local_ip_, local_ip, 16);
    impl_->local_port_ = local_port;

    setCurServiceMessage(conf_message);

    startConnect();
    if (!waitConnected(timeout_sec))
    {
        std::cout << "������������ʧ��" << std::endl;
        return false;
    }
    /// �趨��ȡ���õĶ�ʱʱ�䣨���룩
    setTimer(3000);
    return true;
}

std::string XConfigClient::getString(const char *key)
{
    XMutex mux(&cur_service_conf_mutex);
    if (!cur_service_conf)
        return "";
    /// ��ȡ�ֶ�
    auto field = cur_service_conf->GetDescriptor()->FindFieldByName(key);
    if (!field)
    {
        return "";
    }
    return cur_service_conf->GetReflection()->GetString(*cur_service_conf, field);
}

int XConfigClient::getInt(const char *key)
{
    XMutex mux(&cur_service_conf_mutex);
    if (!cur_service_conf)
        return 0;
    auto field = cur_service_conf->GetDescriptor()->FindFieldByName(key);
    if (!field)
    {
        return 0;
    }
    return cur_service_conf->GetReflection()->GetInt32(*cur_service_conf, field);
}

void XConfigClient::timerCB()
{
    /// ������ȡ���õ�����
    if (impl_->local_port_ > 0)
        loadConfig(impl_->local_ip_, impl_->local_port_);
}

void XConfigClient::setCurServiceMessage(google::protobuf::Message *message)
{
    XMutex mux(&cur_service_conf_mutex);
    cur_service_conf = message;
}
