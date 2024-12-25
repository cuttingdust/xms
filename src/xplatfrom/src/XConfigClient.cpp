#include "XConfigClient.h"

#include <XThreadPool.h>
#include <XTools.h>

/// key ip_port
static std::map<std::string, xmsg::XConfig> conf_map;
static std::mutex                           conf_map_mutex;

/// 存储当前微服务配置
static google::protobuf::Message *cur_service_conf = nullptr;
static std::mutex                 cur_service_conf_mutex;

class XConfigClient::PImpl
{
public:
    PImpl(XConfigClient *owenr);
    ~PImpl() = default;

public:
    XConfigClient *owenr_ = nullptr;
    /// 本地微服务的ip和端口
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
    LOGDEBUG("发送配置");
    sendMsg(xmsg::MT_SAVE_CONFIG_REQ, conf);
}

void XConfigClient::sendConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("接收到上传配置的反馈");
    xmsg::XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("ParseFromArray failed!");
        return;
    }
    if (res.return_() == xmsg::XMessageRes::XR_OK)
    {
        LOGDEBUG("上传配置成功!");
        return;
    }
    std::stringstream ss;
    ss << "上传配置失败:" << res.msg();
    LOGDEBUG(ss.str().c_str());
}

void XConfigClient::loadConfig(const char *ip, int port)
{
    LOGDEBUG("获取配置请求");
    if (port < 0 || port > 65535)
    {
        LOGDEBUG("LoadConfig failed!port error");
        return;
    }
    xmsg::XLoadConfigReq req;
    if (ip) /// IP如果为NULL 则取连接配置中心的地址
        req.set_service_ip(ip);
    req.set_service_port(port);
    /// 发送消息到服务端
    sendMsg(xmsg::MT_LOAD_CONFIG_REQ, &req);
}

void XConfigClient::loadConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("获取配置响应");
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
    /// 更新配置
    conf_map_mutex.lock();
    conf_map[key.str()] = conf;
    conf_map_mutex.unlock();

    /// 存储本地配置
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
    /// s查找配置
    auto conf = conf_map.find(key.str());
    if (conf == conf_map.end())
    {
        LOGDEBUG("Can`t find conf");
        return false;
    }
    /// 复制配置
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
        std::cout << "连接配置中心失败" << std::endl;
        return false;
    }
    /// 设定获取配置的定时时间（毫秒）
    setTimer(3000);
    return true;
}

std::string XConfigClient::getString(const char *key)
{
    XMutex mux(&cur_service_conf_mutex);
    if (!cur_service_conf)
        return "";
    /// 获取字段
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
    /// 发出获取配置的请求
    if (impl_->local_port_ > 0)
        loadConfig(impl_->local_ip_, impl_->local_port_);
}

void XConfigClient::setCurServiceMessage(google::protobuf::Message *message)
{
    XMutex mux(&cur_service_conf_mutex);
    cur_service_conf = message;
}
