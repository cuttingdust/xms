/**
 * @file   XRegisterClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-17
 */

#ifndef XREGISTERCLIENT_H
#define XREGISTERCLIENT_H

#include "XPlatfrom_Global.h"
#include "XServiceClient.h"

class XPLATFROM_EXPORT XRegisterClient : public XServiceClient
{
public:
    static XRegisterClient *get()
    {
        /// TODO 内部存在指针清理行为
        static std::once_flag   s_flag;
        static XRegisterClient *r = nullptr;
        std::call_once(s_flag,
                       [&]()
                       {
                           if (!r)
                               r = new XRegisterClient;
                       });
        return r;
    }

private:
    XRegisterClient();
    ~XRegisterClient() override;

public:
    void connectCB() override;

public:
    /// \brief 向注册中心注册服务 此函数，需要第一个调用，建立连接
    /// \param service_name  微服务名称
    /// \param port          微服务接口
    /// \param ip            微服务IP 如果传递NULL，则采用客户端连接地址
    void registerServer(const char *service_name, int port, const char *ip);

    /// \brief 接收服务的注册响应
    /// \param head
    /// \param msg
    void registerRes(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief 发出有获取微服务列表的请求
    /// \param service_name service_name == NULL 则取全部
    void getServiceReq(const char *service_name);

    /// \brief 获取服务列表的响应
    /// \param head
    /// \param msg
    void getServiceRes(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief   获取所有的服务列表，复制原数据，每次清理上次的复制数据
    /// \return  此函数和操作XServiceMap数据的函数在一个线程
    auto getAllService() const -> xmsg::XServiceMap *;

    /// \brief 获取指定服务名称的微服务列表 （阻塞函数）
    /// 1 等待连接成功 2 发送获取微服务的消息 3 等待微服务列表消息反馈（有可能拿到上一次的配置）
    /// \param service_name 服务名称
    /// \param timeout_sec 超时时间
    /// \return 服务列表
    auto getServices(const char *service_name, int timeout_sec) -> xmsg::XServiceMap::XServiceList;

    void regMsgCallback();

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XREGISTERCLIENT_H
