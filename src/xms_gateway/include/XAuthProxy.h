/**
 * @file   XAuthProxy.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-14
 */

#ifndef XAUTHPROXY_H
#define XAUTHPROXY_H

#include <XServiceProxyClient.h>

class XAuthProxy : public XServiceProxyClient
{
public:
    XAuthProxy();
    ~XAuthProxy() override;

public:
    static auto initAuth() -> void;

    static auto checkToken(const xmsg::XMsgHead *head) -> bool;

    auto readCB(xmsg::XMsgHead *head, XMsg *msg) -> void override;
};


#endif // XAUTHPROXY_H
