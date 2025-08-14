/**
 * @file   XMySSLServiceHandle.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-04-22
 */

#ifndef XMYSSLSERVICEHANDLE_H
#define XMYSSLSERVICEHANDLE_H

#include <XServiceHandle.h>

class XMySSLServiceHandle : public XServiceHandle
{
public:
    XMySSLServiceHandle();
    ~XMySSLServiceHandle() override;

public:
    static void regMsgCallback();

    void loginReq(xmsg::XMsgHead *head, XMsg *msg);

    void connectCB() override;
};


#endif // XMYSSLSERVICEHANDLE_H
