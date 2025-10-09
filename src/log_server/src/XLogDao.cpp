#include "XLogDao.h"

#include <XMsgCom.pb.h>

#include <XTools.h>
#include <LXMysql.h>

constexpr auto table_name       = "xms_log";
constexpr auto col_id           = "id";
constexpr auto col_service_name = "service_name";
constexpr auto col_service_port = "service_port";
constexpr auto col_service_ip   = "service_ip";
constexpr auto col_log_txt      = "log_txt";
constexpr auto col_log_time     = "log_time";
constexpr auto col_log_level    = "log_level";
constexpr auto chart            = "gbk";

static std::mutex log_mutex;

class XLogDao::PImpl
{
public:
    PImpl(XLogDao *owenr);
    ~PImpl() = default;

public:
    XLogDao *owenr_ = nullptr;
    LXMysql *mysql_ = nullptr;
};

XLogDao::PImpl::PImpl(XLogDao *owenr) : owenr_(owenr)
{
}

XLogDao::XLogDao()
{
    impl_ = std::make_unique<XLogDao::PImpl>(this);
}

XLogDao::~XLogDao()
{
}

auto XLogDao::init() -> bool
{
    XMutex mux(&log_mutex);
    if (!impl_->mysql_)
        impl_->mysql_ = new LXMysql();
    if (!impl_->mysql_->init())
    {
        std::cout << "my_->Init() failed!" << std::endl;
        return false;
    }

    impl_->mysql_->setReconnect(true);
    impl_->mysql_->setConnectTimeout(3);

    if (!impl_->mysql_->inputDBConfig())
    {
        std::cout << "my_->inputDBConfig failed!" << std::endl;
        return false;
    }
    std::cout << "my_->inputDBConfig success!" << std::endl;
    return impl_->mysql_->query(std::format("SET NAMES {}", chart).c_str());
}

auto XLogDao::install() -> bool
{
    std::cout << "XLogDao::install()" << std::endl;
    XMutex mux(&log_mutex);
    if (!impl_->mysql_)
    {
        std::cout << "mysql not init" << std::endl;
        return false;
    }

    XFIELDS fields = {
        {
                .name              = col_id,
                .type              = LX_DATA_TYPE::LXD_TYPE_INT24,
                .length            = 0,
                .is_key            = true,
                .is_auto_increment = true,
        },                                                                                 /// id
        { .name = col_service_name, .type = LX_DATA_TYPE::LXD_TYPE_STRING, .length = 16 }, /// 服务端名称
        { .name = col_service_port, .type = LX_DATA_TYPE::LXD_TYPE_INT24, .length = 0 },   /// 服务端端口
        { .name = col_service_ip, .type = LX_DATA_TYPE::LXD_TYPE_STRING, .length = 16 },   /// 服务器IP
        { .name = col_log_txt, .type = LX_DATA_TYPE::LXD_TYPE_STRING, .length = 4096 },    /// 日志内容
        { .name = col_log_time, .type = LX_DATA_TYPE::LXD_TYPE_INT24, .length = 0 },       /// 日志时间戳
        { .name = col_log_level, .type = LX_DATA_TYPE::LXD_TYPE_INT24, .length = 0 },      /// 日志等级
    };

    if (!impl_->mysql_->createTable(table_name, fields, true))
    {
        std::cout << "CREATE TABLE xms_log failed!" << std::endl;
        return false;
    }
    return true;
}

auto XLogDao::addLog(const xmsg::XAddLogReq *req) -> bool
{
    if (!req)
        return false;
    XDATA data;
    data[col_service_name] = req->service_name().c_str();
    data[col_service_ip]   = req->service_ip().c_str();
    int service_port       = req->service_port();
    data[col_service_port] = &service_port;
    data[col_log_txt]      = req->log_txt().c_str();
    int log_time           = req->log_time();
    data[col_log_time]     = &log_time;
    int log_level          = req->log_level();
    data[col_log_level]    = &log_level;

    {
        XMutex mux(&log_mutex);
        return impl_->mysql_->insertBin(data, table_name);
    }
}
