#include <XRegisterClient.h>
#include <XConfigClient.h>

#include <thread>
#include <print>

void ConfigTimer()
{
    static std::string conf_ip   = "";
    static int         conf_port = 0;
    /////////////////////////////////////////////////////////////////
    /// 读取配置项
    std::cout << "config root = " << ConfigClient->GetString("root") << std::endl;

    if (conf_port <= 0)
    {
        /// 从注册中心获取配置中心的IP
        auto confs = RegisterClient->getServices(CONFIG_NAME, 1);
        std::cout << confs.DebugString();
        if (confs.services_size() <= 0)
            return;
        auto conf = confs.services()[0];

        if (conf.ip().empty() || conf.port() <= 0)
            return;
        conf_ip   = conf.ip();
        conf_port = conf.port();
        ConfigClient->setServerIp(conf_ip.c_str());
        ConfigClient->setServerPort(conf_port);
        ConfigClient->connect();
    }
}

int main(int argc, char *argv[])
{
    //////////////////////////////////////////////////////////////////
    int client_port = 4000;

    /// 设置注册中心的IP和端口
    RegisterClient->setServerIp("127.0.0.1");
    RegisterClient->setServerPort(REGISTER_PORT);
    RegisterClient->registerServer("test_config", client_port, 0);
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // /// 从注册中心获取配置中心列表

    /// 初始化配置中心
    xmsg::XDirConfig tmp_conf;
    ConfigClient->startGetConf(0, client_port, &tmp_conf, ConfigTimer);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));


    // auto confs = RegisterClient->getServices(CONFIG_NAME, 1);
    // std::print("{}", confs.DebugString());
    //
    // if (confs.services_size() <= 0)
    //     return -1;
    //
    // auto conf = confs.services()[0];
    // if (conf.ip().empty() || conf.port() <= 0)
    //     return -1;
    //
    // xmsg::XDirConfig tmp_conf;
    // ConfigClient->startGetConf(conf.ip().c_str(), conf.port(), 0, client_port, &tmp_conf);
    //
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    {
        ///////////////////////////////////////////////////////////////////
        /// 存储配置项
        std::string proto;
        auto        message = ConfigClient->loadProto("XDirConfig.proto", "XDirConfig", proto);
        /// 通过反射设置值
        auto ref   = message->GetReflection();
        auto field = message->GetDescriptor()->FindFieldByName("root");
        ref->SetString(message, field, "/test_new_root/");
        std::print("{}", message->DebugString());

        /// 存储配置
        xmsg::XConfig save_conf;
        save_conf.set_service_name("test_config");
        save_conf.set_service_port(client_port);
        save_conf.set_proto(proto);
        save_conf.set_private_pb(message->SerializeAsString());
        ConfigClient->sendConfig(&save_conf);
    }

    // {
    //     /////////////////////////////////////////////////////////////////
    //     ///读取配置项
    //     std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    //     std::cout << "root = " << ConfigClient->GetString("root") << std::endl;
    // }

    {
        /// 读取配置列表 （管理工具）
        for (;;)
        {
            /// 获取配置列表
            auto configs = ConfigClient->getAllConfig(1, 1000, 10);
            std::cout << configs.DebugString();
            if (configs.config_size() <= 0)
            {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }

            /// 取得单个配置信息(第一个配置项)
            std::string ip   = configs.config()[1].service_ip();
            int         port = configs.config()[1].service_port();
            ConfigClient->loadConfig(ip.c_str(), port);
            xmsg::XConfig conf;
            ConfigClient->getConfig(ip.c_str(), port, &conf);
            std::cout << "===========================================" << std::endl;
            std::cout << conf.DebugString() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    //////////////////////////////////////////////////////////////////

    // XConfigClient::get()->regMsgCallback();
    // XConfigClient::get()->setServerIp("127.0.0.1");
    // XConfigClient::get()->setServerPort(CONFIG_PORT);
    // XConfigClient::get()->startConnect();
    // if (!XConfigClient::get()->waitConnected(10))
    // {
    //     std::cout << "link config center failed!" << std::endl;
    // }
    // std::cout << "link config center success." << std::endl;
    // xmsg::XConfig conf;
    // conf.set_service_name("test_client_name");
    // conf.set_service_ip("127.0.0.1");
    // conf.set_service_port(20030);
    // conf.set_proto("message");
    // conf.set_private_pb("test 1pb1");
    // XConfigClient::get()->sendConfig(&conf);
    //
    // XConfigClient::get()->loadConfig("127.0.0.1", 20030);
    //
    // std::this_thread::sleep_for(std::chrono::milliseconds(300));
    // xmsg::XConfig tmp_conf;
    // XConfigClient::get()->getConfig("127.0.0.1", 20030, &tmp_conf);
    // std::cout << "========tmp_conf =========== " << std::endl << tmp_conf.DebugString() << std::endl;
    //
    // const auto configs = XConfigClient::get()->getAllConfig(1, 3, 10);
    // std::cout << configs.DebugString();

    XConfigClient::get()->wait();
    return 0;
}
