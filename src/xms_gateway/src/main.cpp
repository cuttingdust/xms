#include "XConfigClient.h"
#include "XRouteServer.h"
#include "XServiceProxy.h"

#include <XRegisterClient.h>
#include <XThreadPool.h>
#include <XMsgCom.pb.h>

#include <iostream>

int main(int argc, char *argv[])
{
    /// xms_gateway
    std::cout << "xms_gateway API_GATEWAY_PORT REGISTER_IP REGISTER_PORT" << std::endl;
    int server_port = API_GATEWAY_PORT;
    if (argc > 1)
        server_port = atoi(argv[1]);
    std::cout << "server port is " << server_port << std::endl;
    std::string register_ip = "127.0.0.1";
    if (argc > 2)
        register_ip = argv[2];
    int register_port = REGISTER_PORT;
    if (argc > 3)
        register_port = atoi(argv[3]);

    /// ����ע�����ĵ�IP�Ͷ˿�
    XRegisterClient::get()->setServerIp(register_ip.c_str());
    XRegisterClient::get()->setServerPort(register_port);

    /// ע�ᵽע������
    XRegisterClient::get()->registerServer(API_GATEWAY_NAME, server_port, nullptr);

    /// �ȴ�ע����������
    XRegisterClient::get()->waitConnected(3);
    XRegisterClient::get()->getServiceReq(nullptr);

    XServiceProxy::get()->init();

    /// �����Զ�����
    XServiceProxy::get()->start();

    /// �����������ģ���ȡ·������
    /// �ȴ����û�ȡ�ɹ�

    auto confs = XRegisterClient::get()->getServices(CONFIG_NAME, 10);
    std::cout << "=================================================" << std::endl;
    std::cout << confs.DebugString() << std::endl;
    /// ��������IP��ȡʧ�ܣ���ȡ��������
    if (confs.services_size() <= 0)
    {
        std::cout << "find config service failed!" << std::endl;
    }
    else
    {
        /// ֻȡ��һ����������
        auto                        conf = confs.services()[0];
        static xmsg::XGatewayConfig cur_conf;
        if (XConfigClient::get()->startGetConf(conf.ip().c_str(), conf.port(), 0, server_port, &cur_conf))
            std::cout << "�����������ĳɹ�" << cur_conf.DebugString() << std::endl;
    }

    XRouteServer service;
    service.setServerPort(server_port);
    service.start();
    XThreadPool::wait();
    return 0;
}
