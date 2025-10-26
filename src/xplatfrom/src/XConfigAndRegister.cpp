#include "XConfigAndRegister.h"

#include <XConfigClient.h>
#include <XRegisterClient.h>

#define REG  XRegisterClient::get()
#define CONF XConfigClient::get()
static void ConfTimer()
{
    static std::string conf_ip   = "";
    static int         conf_port = 0;
    /////////////////////////////////////////////////////////////////
    //读取配置项
    //cout << "config root = " << CONF->GetString("root") << endl;

    if (conf_port <= 0)
    {
        /// 从注册中心获取配置中心的IP
        auto confs = REG->getServices(CONFIG_NAME, 1);
        std::cout << confs.DebugString();
        if (confs.services_size() <= 0)
        {
            return;
        }
        auto conf = confs.services()[0];

        if (conf.ip().empty() || conf.port() <= 0)
        {
            return;
        }

        conf_ip   = conf.ip();
        conf_port = conf.port();
        CONF->setServerIP(conf_ip.c_str());
        CONF->setServerPort(conf_port);
        CONF->connect();
    }
}


auto XConfigAndRegister::init(const char *service_name, const char *service_ip, int service_port,
                              const char *register_ip, int register_port, google::protobuf::Message *conf_message)
        -> bool
{
    /// 设置注册中心的IP和端口
    XRegisterClient::get()->setServerIP(register_ip);
    XRegisterClient::get()->setServerPort(register_port);

    /// 注册到注册中心
    XRegisterClient::get()->registerServer(AUTH_NAME, service_port, service_ip);

    /// 初始化配置中心
    // XDirConfig tmp_conf;
    CONF->startGetConf(0, service_port, conf_message, ConfTimer);

    return true;
}
