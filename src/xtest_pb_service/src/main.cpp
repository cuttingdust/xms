#include "XDirServiceHandle.h"


#include <XRegisterClient.h>
#include <XConfigClient.h>
#include <XService.h>
#include <XThreadPool.h>
#include <XTools.h>


class XTestService : public XService
{
public:
    XServiceHandle *createHandle() override
    {
        return new XDirServiceHandle();
    }
};


int main(int argc, char *argv[])
{
    std::cout << "test_pb_service SERVER_PORT REGISTER_IP REGISTER_PORT" << std::endl;
    int server_port = 20011;
    if (argc > 1)
        server_port = atoi(argv[1]);
    std::cout << "server port is " << server_port << std::endl;

    std::string register_ip = "127.0.0.1";
    if (argc > 2)
        register_ip = argv[2];
    int register_port = REGISTER_PORT;
    if (argc > 3)
        register_port = atoi(argv[3]);

    /// 设置注册中心的IP和端口
    XRegisterClient::get()->setServerIp(register_ip.c_str());
    XRegisterClient::get()->setServerPort(register_port);
    /// 注册到注册中心
    XRegisterClient::get()->registerServer("dir", server_port, nullptr);

    /// 等待配置获取成功
    const auto &conf_s = XRegisterClient::get()->getServices(CONFIG_NAME, 10);
    std::cout << "=================================================" << std::endl;
    std::cout << conf_s.DebugString() << std::endl;
    if (conf_s.services_size() <= 0)
    {
        std::cout << "find config service failed!" << std::endl;
    }
    else
    {
        /// 只取第一个配置中心
        auto conf = conf_s.services()[0];
        // XConfigClient::get()->regMsgCallback();
        // XConfigClient::get()->setServerIp(conf.ip().c_str());
        // XConfigClient::get()->setServerPort(conf.port());
        // XConfigClient::get()->startConnect();
        // if (!XConfigClient::get()->waitConnected(10))
        // {
        //     std::cout << "连接配置中心失败" << std::endl;
        // }
        static xmsg::XDirConfig cur_dir_conf;
        if (XConfigClient::get()->startGetConf(conf.ip().c_str(), conf.port(), "127.0.0.1", server_port, &cur_dir_conf))
            std::cout << "连接配置中心成功" << cur_dir_conf.DebugString() << std::endl;


        /// 写入测试的配置
        xmsg::XConfig up_conf;
        up_conf.set_service_name("dir");
        up_conf.set_service_ip("127.0.0.1");
        up_conf.set_service_port(server_port);

        /// 类型描述
        xmsg::XDirConfig dir_conf;
        dir_conf.set_root("./");
        up_conf.set_proto(dir_conf.GetDescriptor()->DebugString());
        std::string dir_conf_pb = dir_conf.SerializeAsString();
        up_conf.set_private_pb(dir_conf_pb);
        XConfigClient::get()->sendConfig(&up_conf);


        XConfigClient::get()->loadConfig("127.0.0.1", server_port);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        xmsg::XConfig tmp_conf;
        while (!XConfigClient::get()->getConfig("127.0.0.1", server_port, &tmp_conf))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            std::cout << "0" << std::flush;
        }

        std::cout << "tmp_conf = " << tmp_conf.DebugString() << std::endl;

        /// 反序列化下载的配置
        xmsg::XDirConfig down_conf;
        /// 每个微服务单独的配置
        if (down_conf.ParseFromString(tmp_conf.private_pb()))
            std::cout << "down_conf = " << down_conf.DebugString() << std::endl;
    }

    XDirServiceHandle::regMsgCallback();

    XTestService service;
    service.setServerPort(server_port);
    service.start();
    XThreadPool::wait();

    return 0;
}
