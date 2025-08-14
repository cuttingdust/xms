/**
 * @file   XDirServiceHandle.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-26
 */

#ifndef XDIRSERVICEHANDLE_H
#define XDIRSERVICEHANDLE_H
#include <XServiceHandle.h>

class XDirServiceHandle : public XServiceHandle
{
public:
    XDirServiceHandle();
    ~XDirServiceHandle() override;

    ///处理用户的目录请求
    void dirReq(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief 注册消息回调函数
    static void regMsgCallback();
};


#endif // XDIRSERVICEHANDLE_H
