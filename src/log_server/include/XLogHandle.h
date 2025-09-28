/**
 * @file   XLogHandle.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-09-28
 */

#ifndef XLOGHANDLE_H
#define XLOGHANDLE_H

#include <XServiceHandle.h>

class XLogHandle : public XServiceHandle
{
public:
    XLogHandle();
    ~XLogHandle() override;

public:
    auto addLogReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    static auto regMsgCallback() -> void;
};


#endif // XLOGHANDLE_H
