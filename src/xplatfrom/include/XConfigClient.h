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

    static void regMsgCallback();

    void wait();
};


#endif // XCONFIGCLIENT_H
