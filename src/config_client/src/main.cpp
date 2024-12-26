#include <iostream>

#include "XConfigClient.h"

#include <thread>

int main(int argc, char *argv[])
{
    XConfigClient::get()->regMsgCallback();
    XConfigClient::get()->setServerIp("127.0.0.1");
    XConfigClient::get()->setServerPort(CONFIG_PORT);
    XConfigClient::get()->startConnect();
    if (!XConfigClient::get()->waitConnected(10))
    {
        std::cout << "连接配置中心失败" << std::endl;
    }
    std::cout << "连接配置中心成功" << std::endl;
    xmsg::XConfig conf;
    conf.set_service_name("test_client_name");
    conf.set_service_ip("127.0.0.1");
    conf.set_service_port(20030);
    conf.set_proto("message");
    conf.set_private_pb("test 1pb1");
    XConfigClient::get()->sendConfig(&conf);

    XConfigClient::get()->loadConfig("127.0.0.1", 20030);

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    xmsg::XConfig tmp_conf;
    XConfigClient::get()->getConfig("127.0.0.1", 20030, &tmp_conf);
    std::cout << "========tmp_conf =========== " << std::endl << tmp_conf.DebugString() << std::endl;

    const auto configs = XConfigClient::get()->getAllConfig(1, 3, 10);
    std::cout << configs.DebugString();

    XConfigClient::get()->wait();
    return 0;
}
