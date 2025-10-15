/**
 * @file   XDirHandle.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-13
 */

#ifndef XDIRHANDLE_H
#define XDIRHANDLE_H

#include <XServiceHandle.h>

class XDirHandle : public XServiceHandle
{
public:
    XDirHandle();
    ~XDirHandle() override;

public:
    auto getDirReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto newDirReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    static auto regMsgCallback() -> void;
};


#endif // XDIRHANDLE_H
