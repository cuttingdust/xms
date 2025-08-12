#include <print>

#include "XRegisterClient.h"
#include "XConfigClient.h"

#include <thread>

void ConfigTimer()
{
    static std::string conf_ip   = "";
    static int         conf_port = 0;
    /////////////////////////////////////////////////////////////////
    /// ��ȡ������
    std::cout << "config root = " << ConfigClient->GetString("root") << std::endl;

    if (conf_port <= 0)
    {
        /// ��ע�����Ļ�ȡ�������ĵ�IP
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

    /// ����ע�����ĵ�IP�Ͷ˿�
    RegisterClient->setServerIp("127.0.0.1");
    RegisterClient->setServerPort(REGISTER_PORT);
    RegisterClient->registerServer("test_config", client_port, 0);
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // /// ��ע�����Ļ�ȡ���������б�

    /// ��ʼ����������
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
        /// �洢������
        std::string proto;
        auto        message = ConfigClient->loadProto("XDirConfig.proto", "XDirConfig", proto);
        /// ͨ����������ֵ
        auto ref   = message->GetReflection();
        auto field = message->GetDescriptor()->FindFieldByName("root");
        ref->SetString(message, field, "/test_new_root/");
        std::print("{}", message->DebugString());

        /// �洢����
        xmsg::XConfig save_conf;
        save_conf.set_service_name("test_config");
        save_conf.set_service_port(client_port);
        save_conf.set_proto(proto);
        save_conf.set_private_pb(message->SerializeAsString());
        ConfigClient->sendConfig(&save_conf);
    }

    // {
    //     /////////////////////////////////////////////////////////////////
    //     ///��ȡ������
    //     std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    //     std::cout << "root = " << ConfigClient->GetString("root") << std::endl;
    // }

    {
        /// ��ȡ�����б� �������ߣ�
        for (;;)
        {
            /// ��ȡ�����б�
            auto configs = ConfigClient->getAllConfig(1, 1000, 10);
            std::cout << configs.DebugString();
            if (configs.config_size() <= 0)
            {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }

            /// ȡ�õ���������Ϣ(��һ��������)
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
