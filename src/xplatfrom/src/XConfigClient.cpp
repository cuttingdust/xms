#include "XConfigClient.h"

#include <XThreadPool.h>
#include <XTools.h>
#include <XMsgCom.pb.h>

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>

#define PB_ROOT "root/"

/// key ip_port
static std::map<std::string, xmsg::XConfig> conf_map;
static std::mutex                           conf_map_mutex;
static std::condition_variable              config_cv;
static bool                                 config_received = false;

/// 存储当前微服务配置
static google::protobuf::Message *cur_service_conf = nullptr;
static std::mutex                 cur_service_conf_mutex;

/// 存储获取的配置列表
static xmsg::XConfigList *all_config = nullptr;
static std::mutex         all_config_mutex;

/// 显示解析的语法错误
class ConfError : public google::protobuf::compiler::MultiFileErrorCollector
{
public:
    void RecordError(absl::string_view filename, int line, int column, absl::string_view message) override
    {
        std::stringstream ss;
        ss << filename << "|" << line << "|" << column << "|" << message;
        LOGDEBUG(ss.str().c_str());
    }
};

static ConfError config_error;

class XConfigClient::PImpl
{
public:
    PImpl(XConfigClient *owenr);
    ~PImpl() = default;

public:
    XConfigClient                              *owenr_        = nullptr;
    char                                        local_ip_[16] = { 0 };   /// 本地微服务的ip
    int                                         local_port_   = 0;       /// 本地微服务的端口
    google::protobuf::compiler::Importer       *importer_     = nullptr; /// 动态解析proto文件
    google::protobuf::compiler::DiskSourceTree *source_tree_  = nullptr; /// 解析文件的管理对象
    google::protobuf::Message                  *message_      = nullptr; /// 根据proto文件动态创建的的message
};

XConfigClient::PImpl::PImpl(XConfigClient *owenr) : owenr_(owenr)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    /// 文件加载路径
    source_tree_ = new google::protobuf::compiler::DiskSourceTree();
    source_tree_->MapPath("", "");
    /// 使用绝对路径时，不加root会失败
    source_tree_->MapPath(PB_ROOT, "");
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
        if (sendConfigResCB)
            sendConfigResCB(false, "ParseFromArray failed!");
        return;
    }
    if (res.return_() == xmsg::XMessageRes::XR_OK)
    {
        LOGDEBUG("上传配置成功!");
        if (sendConfigResCB)
            sendConfigResCB(true, "上传配置成功!");
        return;
    }
    std::stringstream ss;
    ss << "上传配置失败:" << res.msg();
    if (sendConfigResCB)
        sendConfigResCB(false, ss.str().c_str());
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
    conf_map[key.str()] = conf;
    config_cv.notify_one();

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
    std::unique_lock<std::mutex> lck(conf_map_mutex);
    config_cv.wait(lck);

    std::stringstream key;
    key << ip << "_" << port;
    /// s查找配置
    auto conf = conf_map.find(key.str());
    if (conf == conf_map.end())
    {
        LOGDEBUG("Can`t find conf");
        return false;
    }
    LOGDEBUG(conf->second.DebugString());
    /// 复制配置
    out_conf->CopyFrom(conf->second);
    return true;
}

google::protobuf::Message *XConfigClient::loadProto(const std::string &file_name, const std::string &class_name,
                                                    std::string &out_proto_code)
{
    if (impl_->importer_)
    {
        delete impl_->importer_;
        impl_->importer_ = nullptr;
    }

    impl_->importer_ = new google::protobuf::compiler::Importer(impl_->source_tree_, &config_error);
    if (!impl_->importer_)
    {
        return nullptr;
    }
    /// 1 加载proto文件
    std::string path = PB_ROOT;
    path += file_name;

    /// 返回proto文件描述符
    auto file_desc = impl_->importer_->Import(path);
    if (!file_desc)
    {
        return nullptr;
    }
    LOGDEBUG(file_desc->DebugString());
    std::stringstream ss;
    ss << file_name << "proto 文件加载成功";
    LOGDEBUG(ss.str().c_str());


    /// 获取类型描述符
    /// 如果class_name为空，则使用第一个类型
    const google::protobuf::Descriptor *message_desc = nullptr;
    if (class_name.empty())
    {
        if (file_desc->message_type_count() <= 0)
        {
            LOGDEBUG("proto文件中没有message");
            return NULL;
        }
        /// 取第一个类型
        message_desc = file_desc->message_type(0);
    }
    else
    {
        /// 包含命名空间的类名 xmsg.XDirConfig
        std::string class_name_pack;
        /// 查找类型 命名空间，是否要用户提供

        ///用户没有提供命名空间
        if (class_name.find('.') == std::string::npos)
        {
            if (file_desc->package().empty())
            {
                class_name_pack = class_name;
            }
            else
            {
                class_name_pack = file_desc->package();
                class_name_pack += ".";
                class_name_pack += class_name;
            }
        }
        else
        {
            class_name_pack = class_name;
        }
        message_desc = impl_->importer_->pool()->FindMessageTypeByName(class_name_pack);
        if (!message_desc)
        {
            std::string log = "proto文件中没有指定的message ";
            log += class_name_pack;
            LOGDEBUG(log.c_str());
            return nullptr;
        }
    }

    LOGDEBUG(message_desc->DebugString());

    /// 反射生成message对象
    // if (impl_->message_)
    // {
    //     delete impl_->message_;
    //     impl_->message_ = nullptr;
    // }

    /// 动态创建消息类型的工厂，不能销毁，销毁后由此创建的message也失效
    static google::protobuf::DynamicMessageFactory factory;


    /// 创建一个类型原型
    auto message_proto = factory.GetPrototype(message_desc);
    impl_->message_    = message_proto->New();
    LOGDEBUG(impl_->message_->DebugString());

    ////////////////////////////////////////
    /// syntax="proto3";	//版本号
    /// package xmsg;		//命名空间
    /// message XDirConfig
    /// {
    ///     string root = 1;
    /// }
    /////////////////////////////////////////
    // syntax="proto3";	//版本号

    google::protobuf::FileDescriptorProto proto;
    file_desc->CopyTo(&proto);
    std::string syntax_version = proto.syntax();
    out_proto_code             = "syntax=\"";
    out_proto_code += syntax_version;
    out_proto_code += "\";\n";
    //package xmsg;		//命名空间
    out_proto_code += "package ";
    out_proto_code += file_desc->package();
    out_proto_code += ";\n";

    /// 存枚举定义 ，暂时不支持多proto import文件
    /// 同一个类型只生成一次代码
    std::map<std::string, const google::protobuf::EnumDescriptor *> enum_desc;
    for (int i = 0; i < message_desc->field_count(); i++)
    {
        auto field = message_desc->field(i);
        if (field->type() != google::protobuf::FieldDescriptor::TYPE_ENUM)
        {
            continue;
        }
        /// 如果是枚举类型
        if (enum_desc.contains(field->enum_type()->name())) /// 已经添加过的类型
            continue;
        out_proto_code += field->enum_type()->DebugString() + "\n";
        enum_desc[field->enum_type()->name()] = field->enum_type();
    }

    //message XDirConfig
    out_proto_code += message_desc->DebugString();

    return impl_->message_;
}


void XConfigClient::regMsgCallback()
{
    regCB(xmsg::MT_SAVE_CONFIG_RES, static_cast<MsgCBFunc>(&XConfigClient::sendConfigRes));
    regCB(xmsg::MT_LOAD_CONFIG_RES, static_cast<MsgCBFunc>(&XConfigClient::loadConfigRes));
    regCB(xmsg::MT_LOAD_ALL_CONFIG_RES, static_cast<MsgCBFunc>(&XConfigClient::loadAllConfigRes));
    regCB(xmsg::MT_DEL_CONFIG_RES, static_cast<MsgCBFunc>(&XConfigClient::deleteConfigRes));
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
    if (local_ip)
        strncpy(impl_->local_ip_, local_ip, 16);
    impl_->local_port_ = local_port;

    setCurServiceMessage(conf_message);

    startConnect();
    if (!waitConnected(timeout_sec))
    {
        std::cout << "connting config center failed..." << std::endl;
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

void XConfigClient::loadAllConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("Response to get config list.");
    XMutex mux(&all_config_mutex);
    if (!all_config)
        all_config = new xmsg::XConfigList();
    all_config->ParseFromArray(msg->data, msg->size);
}

xmsg::XConfigList XConfigClient::getAllConfig(int page, int page_count, int timeout_sec)
{
    ///清理历史数据
    {
        XMutex mux(&all_config_mutex);
        delete all_config;
        all_config = nullptr;
    }

    xmsg::XConfigList confs;
    /// 1 断开连接自动重连
    if (!autoConnect(timeout_sec))
        return confs;

    /// 2 发送获取配置列表的消息
    xmsg::XLoadAllConfigReq req;
    req.set_page(page);
    req.set_page_count(page_count);
    sendMsg(xmsg::MT_LOAD_ALL_CONFIG_REQ, &req);

    /// 10毫秒监听一次
    int count = timeout_sec * 100;
    for (int i = 0; i < count; i++)
    {
        {
            /// 会在return 之后调用释放
            XMutex mux(&all_config_mutex);
            if (all_config)
            {
                return *all_config;
            }
        }
        /// 是否收到响应
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return confs;
}

void XConfigClient::deleteConfig(const char *ip, int port)
{
    if (!ip || strlen(ip) == 0 || port < 0 || port > 65535)
    {
        LOGDEBUG("DeleteConfig failed!port or ip error");
        return;
    }
    xmsg::XLoadConfigReq req;
    req.set_service_ip(ip);
    req.set_service_port(port);
    /// 发送消息到服务端
    sendMsg(xmsg::MT_DEL_CONFIG_REQ, &req);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void XConfigClient::deleteConfigRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("接收到删除配置的反馈");
    xmsg::XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("ParseFromArray failed!");
        return;
    }
    if (res.return_() == xmsg::XMessageRes::XR_OK)
    {
        LOGDEBUG("删除配置成功!");
        return;
    }
    LOGDEBUG("删除配置失败!");
}
