/**
 * @file   XRegisterHandle.h
 * @brief  处理注册中心的客户端 对应一个连接
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-17
 */

#ifndef XREGISTERHANDLE_H
#define XREGISTERHANDLE_H

#include "XServiceHandle.h"

class XRegisterHandle : public XServiceHandle
{
public:
    void registerReq(xmsg::XMsgHead *head, XMsg *msg);

    static void regMsgCallback();
};


#endif // XREGISTERHANDLE_H
