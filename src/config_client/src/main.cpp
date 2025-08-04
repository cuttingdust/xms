#include <print>

#include "XRegisterClient.h"
#include "XConfigClient.h"

#include <thread>

int main(int argc, char *argv[])
{
    //////////////////////////////////////////////////////////////////
    int client_port = 4000;

    /// 设置注册中心的IP和端口
    RegisterClient->setServerIp("127.0.0.1");
    RegisterClient->setServerPort(REGISTER_PORT);
    RegisterClient->registerServer("test_config", client_port, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    /// 从注册中心获取配置中心列表


    auto confs = RegisterClient->getServices(CONFIG_NAME, 1);
    std::print("{}", confs.DebugString());
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
