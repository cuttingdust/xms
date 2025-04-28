/**
 * @file   XConfigClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-23
 */

#ifndef XCONFIGCLIENT_H
#define XCONFIGCLIENT_H

#include "XPlatfrom_Global.h"
#include <XServiceClient.h>

typedef void (*ConfigResCBFunc)(bool is_ok, const char *msg);

class XPLATFROM_EXPORT XConfigClient : public XServiceClient
{
public:
    static XConfigClient *get()
    {
        static XConfigClient instance;
        return &instance;
    }

private:
    XConfigClient();
    ~XConfigClient() override;

public:
    /// \brief 发送配置
    /// \param conf
    void sendConfig(xmsg::XConfig *conf);

    /// \brief 接收到保存配置的消息
    /// \param head
    /// \param msg
    void sendConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief 获取配置请求 IP如果为NULL 则取连接配置中心的地址
    /// \param ip
    /// \param port
    void loadConfig(const char *ip, int port);

    void loadConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief
    /// \param ip
    /// \param port
    /// \param out_conf
    /// \return
    bool getConfig(const char *ip, int port, xmsg::XConfig *out_conf);

    /// \brief 载入proto文件 线程不安全
    /// \param file_name  文件路径
    /// \param class_name  配置的类型
    /// \param out_proto_code 读取到的代码，包含空间和版本
    /// \return
    google::protobuf::Message *loadProto(const std::string &file_name, const std::string &class_name,
                                         std::string &out_proto_code);

    static void regMsgCallback();

    void wait();

    /// \brief 连接配置中心，开始定时器获取配置
    /// \param server_ip
    /// \param server_port
    /// \param local_ip
    /// \param local_port
    /// \param conf_message
    /// \param timeout_sec
    /// \return
    bool startGetConf(const char *server_ip, int server_port, const char *local_ip, int local_port,
                      google::protobuf::Message *conf_message, int timeout_sec = 10);

    /// \brief 获取下载的本地参数
    /// \return
    std::string getString(const char *key);
    int         getInt(const char *key);
    bool        getBool(const char *key);

    void timerCB() override;

    /// \brief 设置当前配置的对象
    /// \param message
    void setCurServiceMessage(google::protobuf::Message *message);

    void loadAllConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief 获取全部配置列表
    /// 1 断开连接自动重连
    /// 2 等待结果返回
    /// \param page
    /// \param page_count
    /// \param timeout_sec
    /// \return
    xmsg::XConfigList getAllConfig(int page, int page_count, int timeout_sec);

    /// \brief 发出删除配置请求
    /// \param ip
    /// \param port
    void deleteConfig(const char *ip, int port);

    void deleteConfigRes(xmsg::XMsgHead *head, XMsg *msg);

public:
    ConfigResCBFunc sendConfigResCB = nullptr;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XCONFIGCLIENT_H
