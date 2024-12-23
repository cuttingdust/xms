#include "ConfigDao.h"

#include "LXMysql.h"


#include <XTools.h>

#include <iostream>
#include <format>

constexpr auto table_name      = "xms_service_config";
constexpr auto col_id          = "id";
constexpr auto col_server_name = "service_name";
constexpr auto col_server_port = "service_port";
constexpr auto col_server_ip   = "service_ip";
constexpr auto col_private_pb  = "private_pb";
constexpr auto col_proto       = "proto";
constexpr auto chart           = "gbk";

class ConfigDao::PImpl
{
public:
    PImpl(ConfigDao *owenr);
    ~PImpl() = default;

public:
    ConfigDao *owenr_ = nullptr;
    LXMysql   *mysql_ = nullptr;
};

ConfigDao::PImpl::PImpl(ConfigDao *owenr) : owenr_(owenr)
{
}

ConfigDao::ConfigDao()
{
    impl_ = std::make_unique<PImpl>(this);
}

ConfigDao::~ConfigDao()
{
}

bool ConfigDao::init(const char *ip, const char *user, const char *pass, const char *db_name, int port)
{
    if (!impl_->mysql_)
        impl_->mysql_ = new LXMysql();
    if (!impl_->mysql_->init())
    {
        LOGDEBUG("my_->Init() failed!");
        return false;
    }

    impl_->mysql_->setReconnect(true);
    impl_->mysql_->setConnectTimeout(3);
    if (!impl_->mysql_->connect(ip, user, pass, db_name, port, 0, true))
    {
        LOGDEBUG("my_->Connect failed!");
        return false;
    }
    LOGDEBUG("my_->Connect success!");

    return impl_->mysql_->query(std::format("SET NAMES {}", chart).c_str());
}

bool ConfigDao::install()
{
    LOGDEBUG("ConfigDao::Install()");
    if (!impl_->mysql_)
    {
        LOGERROR("mysql not init");
        return false;
    }


    std::string sql = "";

    XFIELDS fields = {
        { col_id, LX_DATA_TYPE::LXD_TYPE_INT24, 0, true, true }, ///< id
        { col_server_name, LX_DATA_TYPE::LXD_TYPE_STRING, 16 },  /// 服务器名称
        { col_server_port, LX_DATA_TYPE::LXD_TYPE_INT24, 0 },    /// 服务器端口
        { col_server_ip, LX_DATA_TYPE::LXD_TYPE_STRING, 16 },    /// 服务器IP
        { col_private_pb, LX_DATA_TYPE::LXD_TYPE_STRING, 4096 }, /// 私有协议
        { col_proto, LX_DATA_TYPE::LXD_TYPE_STRING, 4096 },      /// 公共协议
    };

    if (!impl_->mysql_->createTable(table_name, fields, true))
    {
        LOGINFO("CREATE TABLE xms_service_config failed!");
        return false;
    }

    return true;
}

bool ConfigDao::saveConfig(const xmsg::XConfig *conf)
{
    if (!impl_->mysql_)
    {
        LOGERROR("mysql not init");
        return false;
    }
    if (!conf || conf->service_ip().empty())
    {
        LOGERROR("ConfigDao::SaveConfig failed,conf value error!");
        return false;
    }

    XDATA data;
    data[col_server_name] = conf->service_name().c_str();
    const int port        = conf->service_port();
    data[col_server_port]     = &port;
    data[col_server_ip]       = conf->service_ip().c_str();
    data[col_private_pb] = conf->private_pb().c_str();
    data[col_proto]           = conf->proto().c_str();
    return impl_->mysql_->insertBin(data, table_name);
}
