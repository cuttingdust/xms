﻿/**
 * @file   XServiceProxy.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-11-28
 */

#ifndef XSERVICEPROXY_H
#define XSERVICEPROXY_H
#include <XMsgEvent.h>

class XServiceProxy
{
public:
    static XServiceProxy *get()
    {
        static XServiceProxy xs;
        return &xs;
    }

public:
    /// \brief 清理消息回调
    /// \param ev
    void delEvent(XMsgEvent *ev);

    /// \brief 负载均衡找到客户端连接 进行数据发送
    /// \param head 消息头 含路由信息
    /// \param msg
    /// \param ev
    /// \return
    bool sendMsg(xmsg::XMsgHead *head, XMsg *msg, XMsgEvent *ev);

    /// \brief 开启自动重连的线程
    void start();

    /// \brief 初始化微服务列表(注册中心获取),建立连接
    /// \return
    bool init();

private:
    XServiceProxy();
    virtual ~XServiceProxy();

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XSERVICEPROXY_H
