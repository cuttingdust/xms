#include "XAuthProxy.h"
#include "XRouteServer.h"
#include "XServiceProxy.h"

#include <XTools.h>
#include <XRegisterClient.h>
#include <XConfigClient.h>

#include <iostream>

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
        ConfigClient->setServerIP(conf_ip.c_str());
        ConfigClient->setServerPort(conf_port);
        ConfigClient->connect();
    }
}

int main(int argc, char *argv[])
{
    /// xms_gateway
    std::cout << "xms_gateway API_GATEWAY_PORT REGISTER_IP REGISTER_PORT" << std::endl;
    int server_port = API_GATEWAY_PORT;
    if (argc > 1)
    {
        server_port = atoi(argv[1]);
    }
    std::cout << "server port is " << server_port << std::endl;
    std::string register_ip = XTools::XGetHostByName(API_REGISTER_SERVER_NAME);
    if (argc > 2)
    {
        register_ip = argv[2];
    }
    int register_port = REGISTER_PORT;
    if (argc > 3)
    {
        register_port = atoi(argv[3]);
    }

    if (register_ip.empty())
    {
        register_ip = "127.0.0.1";
    }

    XAuthProxy::initAuth();

    /// 设置注册中心的IP和端口
    XRegisterClient::get()->setServerIP(register_ip.c_str());
    XRegisterClient::get()->setServerPort(register_port);

    /// 注册到注册中心
    XRegisterClient::get()->registerServer(API_GATEWAY_NAME, server_port, nullptr, true);

    /// 等待注册中心连接
    XRegisterClient::get()->waitConnected(3);
    XRegisterClient::get()->getServiceReq(nullptr);

    XServiceProxy::get()->init();

    /// 开启自动重连
    XServiceProxy::get()->start();

    static xmsg::XGatewayConfig cur_conf;
    if (XConfigClient::get()->startGetConf(0, server_port, &cur_conf, ConfigTimer))
    {
        std::cout << "初始化配置中心成功" << cur_conf.DebugString() << std::endl;
    }


    /// 连接配置中心，获取路由配置
    /// 等待配置获取成功

    // auto confs = XRegisterClient::get()->getServices(CONFIG_NAME, 10);
    // std::cout << "=================================================" << std::endl;
    // std::cout << confs.DebugString() << std::endl;
    // /// 配置中心IP获取失败，读取本地配置
    // if (confs.services_size() <= 0)
    // {
    //     std::cout << "find config service failed!" << std::endl;
    // }
    // else
    // {
    //     /// 只取第一个配置中心
    //     auto                        conf = confs.services()[0];
    //     static xmsg::XGatewayConfig cur_conf;
    //     if (XConfigClient::get()->startGetConf(conf.ip().c_str(), conf.port(), 0, server_port, &cur_conf))
    //         std::cout << "连接配置中心成功" << cur_conf.DebugString() << std::endl;
    // }

    XRouteServer service;
    service.setServerPort(server_port);
    service.start();
    service.wait();
    return 0;
}
