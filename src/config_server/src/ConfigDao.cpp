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

static std::mutex my_mutex;

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
    XMutex mux(&my_mutex);

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

    XMutex mux(&my_mutex);
    if (!impl_->mysql_)
    {
        LOGERROR("mysql not init");
        return false;
    }


    std::string sql = "";

    XFIELDS fields = {
        { col_id, LX_DATA_TYPE::LXD_TYPE_INT24, 0, true, true }, ///< id
        { col_server_name, LX_DATA_TYPE::LXD_TYPE_STRING, 16 },  /// ����������
        { col_server_port, LX_DATA_TYPE::LXD_TYPE_INT24, 0 },    /// �������˿�
        { col_server_ip, LX_DATA_TYPE::LXD_TYPE_STRING, 16 },    /// ������IP
        { col_private_pb, LX_DATA_TYPE::LXD_TYPE_STRING, 4096 }, /// ˽��Э��
        { col_proto, LX_DATA_TYPE::LXD_TYPE_STRING, 4096 },      /// ����Э��
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
    LOGDEBUG("ConfigDao::SaveConfig");
    XMutex mux(&my_mutex);

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
    data[col_server_port] = &port;
    data[col_server_ip]   = conf->service_ip().c_str();

    /// �����л�һ�Σ�������XConfig ���뵽private_pb
    std::string private_pb;
    conf->SerializeToString(&private_pb);
    data[col_private_pb] = private_pb.c_str();
    data[col_proto]      = conf->proto().c_str();

    const auto conf_ip   = conf->service_ip();
    const auto conf_port = conf->service_port();
    const auto str_port  = std::to_string(conf_port);

    auto row = impl_->mysql_->getRows(table_name, "*", { { col_server_ip, conf_ip }, { col_server_port, str_port } });
    if (!row.empty())
    {
        int count = impl_->mysql_->updateBin(data, table_name,
                                             { { col_server_ip, conf_ip }, { col_server_port, str_port } });
        if (count >= 0)
        {
            LOGDEBUG("���ø��³ɹ���");
            return true;
        }
        LOGDEBUG("���ø���ʧ�ܣ�");
        return false;
    }
    bool re = impl_->mysql_->insertBin(data, table_name);
    if (re)
    {
        LOGDEBUG("���ò���ɹ���");
    }
    else
    {
        LOGDEBUG("���ò���ʧ�ܣ�");
    }
    return re;
}
xmsg::XConfig ConfigDao::loadConfig(const char *ip, int port)
{
    xmsg::XConfig conf;
    LOGDEBUG("ConfigDao::LoadConfig");

    XMutex mux(&my_mutex);
    if (!impl_->mysql_)
    {
        LOGERROR("mysql not init");
        return conf;
    }
    if (!ip || port <= 0 || port > 65535 || strlen(ip) == 0)
    {
        LOGERROR("LoadConfig failed!ip or port error");
        return conf;
    }
    auto rows = impl_->mysql_->getRows(table_name, col_private_pb,
                                       { { col_server_ip, ip }, { col_server_port, std::to_string(port) } });

    if (rows.empty())
    {
        LOGDEBUG("download config failed!");
        return conf;
    }
    /// ֻȡ��һ��
    auto row = rows[0];
    if (!conf.ParseFromArray(row[0].data, row[0].size))
    {
        LOGDEBUG("download config failed! ParseFromArray failed!");
        return conf;
    }
    LOGDEBUG("download config success!");
    LOGDEBUG(conf.DebugString());
    return conf;
}

xmsg::XConfigList ConfigDao::loadAllConfig(unsigned int page, int page_count)
{
    xmsg::XConfigList confs;
    LOGDEBUG("ConfigDao::LoadAllConfig");
    XMutex mux(&my_mutex);
    if (!impl_->mysql_)
    {
        LOGERROR("mysql not init");
        return confs;
    }
    if (page <= 0 || page_count <= 0)
    {
        LOGERROR("LoadAllConfig para erorr");
        return confs;
    }

    std::vector<std::string> sel = { col_server_name, col_server_ip, col_server_port };
    auto rows = impl_->mysql_->getRows(table_name, sel, std::make_pair("", ""), { page, page_count });
    for (auto row : rows)
    {
        /// ������������뵽proto������
        auto conf = confs.add_config();
        conf->set_service_name(row[0].data);
        conf->set_service_ip(row[1].data);
        conf->set_service_port(atoi(row[2].data));
    }
    return confs;
}
