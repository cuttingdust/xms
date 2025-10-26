/**
 * @file   XServiceProxyClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-11-28
 */

#ifndef XSERVICEPROXYCLIENT_H
#define XSERVICEPROXYCLIENT_H

#include <XServiceClient.h>

class XServiceProxyClient : public XServiceClient
{
public:
    static auto create(std::string service_name) -> XServiceProxyClient *;

protected:
    XServiceProxyClient();
    ~XServiceProxyClient() override;

public:
    auto setFindFlag(bool bFind) -> void;

    auto isFind() const -> bool;

    auto sendMsg(xmsg::XMsgHead *head, XMsg *msg, XMsgEvent *ev) -> bool;

    auto delEvent(XMsgEvent *ev) -> void;

    auto regEvent(XMsgEvent *ev) -> void;

    auto readCB(xmsg::XMsgHead *head, XMsg *msg) -> void override;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};

#endif // XSERVICEPROXYCLIENT_H
