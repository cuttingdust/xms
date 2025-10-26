/**
 * @file   XDownloadHandle.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-27
 */

#ifndef XDOWNLOADHANDLE_H
#define XDOWNLOADHANDLE_H

#include <XServiceHandle.h>

class XDownloadHandle : public XServiceHandle
{
public:
    XDownloadHandle();
    ~XDownloadHandle() override;

public:
    static auto regMsgCallback() -> void;

    auto downloadFileReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto downloadFileBegin(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto downloadSliceRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XDOWNLOADHANDLE_H
