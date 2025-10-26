/**
 * @file   XServiceClient.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-25
 */

#ifndef XSERVICECLIENT_H
#define XSERVICECLIENT_H

#include "XPlatfrom_Global.h"
#include "XMsgEvent.h"

class XPLATFROM_EXPORT XServiceClient : public XMsgEvent
{
public:
    XServiceClient();
    ~XServiceClient() override;

public:
    virtual auto setLogin(const xmsg::XLoginRes *login) -> void;

    virtual auto setServiceName(const std::string &serviceName) -> void;

    virtual auto getServiceName() const -> std::string;

    auto setHead(xmsg::XMsgHead *head) -> xmsg::XMsgHead *;

    /// \brief 发送消息
    /// \param msgType
    /// \param msg
    /// \return
    auto sendMsg(const xmsg::MsgType &msgType, const google::protobuf::Message *msg) -> bool override;

    /// \brief 发送消息
    /// \param h
    /// \param msg
    /// \return
    auto sendMsg(xmsg::XMsgHead *h, const google::protobuf::Message *msg) -> bool override;

    /// \brief 发送文件用
    /// \param head
    /// \param msg
    /// \return
    auto sendMsg(xmsg::XMsgHead *head, XMsg *msg) -> bool override;

public:
    /// \brief 将任务加入到线程池中，进行连接
    virtual auto startConnect() -> void;

    virtual auto wait() -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XSERVICECLIENT_H
