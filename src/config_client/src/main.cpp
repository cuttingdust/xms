#include <iostream>

#include "XConfigClient.h"

int main(int argc, char *argv[])
{
    XConfigClient::get()->regMsgCallback();
    XConfigClient::get()->setServerIp("127.0.0.1");
    XConfigClient::get()->setServerPort(CONFIG_PORT);
    XConfigClient::get()->startConnect();
    if (!XConfigClient::get()->waitConnected(10))
    {
        std::cout << "������������ʧ��" << std::endl;
    }
    std::cout << "�����������ĳɹ�" << std::endl;
    xmsg::XConfig conf;
    conf.set_service_name("test_client_name");
    conf.set_service_ip("127.0.0.1");
    conf.set_service_port(20030);
    conf.set_proto("message");
    conf.set_private_pb("test 1pb1");
    XConfigClient::get()->sendConfig(&conf);

    XConfigClient::get()->loadConfig("127.0.0.1", 20030);

    XConfigClient::get()->wait();
    return 0;
}
