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
    XServiceProxyClient();
    ~XServiceProxyClient() override;

public:
    bool sendMsg(xmsg::XMsgHead *head, XMsg *msg, XMsgEvent *ev);

    void delEvent(XMsgEvent *ev);

    void regEvent(XMsgEvent *ev);

    void readCB(xmsg::XMsgHead *head, XMsg *msg) override;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};

#endif // XSERVICEPROXYCLIENT_H
