#include "ConfigDao.h"
#include "XMsgCom.pb.h"

#include <XTools.h>
#include <XConfigServer.h>

#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "Config Server" << std::endl;

    if (ConfigDao::get()->init("localhost", "root", "System123@", "xms", 3306))
    {
        std::cout << "ConfigDao::Get()->Init Success!" << std::endl;
        ConfigDao::get()->install();
    }
    else
    {
        std::cout << "ConfigDao::Get()->Init Failed!" << std::endl;
    }

    auto re = ConfigDao::get()->loadAllConfig(1, 10000);
    std::cout << re.DebugString();

    XConfigServer config;
    config.main(argc, argv);
    config.start();
    config.wait();

    // /// 先测试DAO
    // /// 初始化数据库
    // if (ConfigDao::get()->init("localhost", "root", "System123@", "xms", 3306))
    // {
    //     std::cout << "ConfigDao::Get()->Init Success!" << std::endl;
    //     /// 测试安装
    //     ConfigDao::get()->install();
    //
    //     /// 测试配置保持
    //     xmsg::XConfig conf;
    //     conf.set_service_name("test");
    //     conf.set_service_ip("127.0.0.1");
    //     conf.set_service_port(20020);
    //     conf.set_proto("message Test{string name=1;}");
    //     std::string pb = conf.SerializeAsString();
    //     conf.set_private_pb(pb);
    //
    //     ConfigDao::get()->saveConfig(&conf);
    //     auto config = ConfigDao::get()->loadConfig("127.0.0.1", 20020);
    //     std::cout << "========================================" << std::endl;
    //     LOGDEBUG(config.DebugString());
    // }
    // else
    // {
    //     std::cout << "ConfigDao::Get()->Init Failed!" << std::endl;
    // }

    std::cin.get();
    return 0;
}
