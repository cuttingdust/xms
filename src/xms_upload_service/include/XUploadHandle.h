/**
 * @file   XUploadHandle.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-20
 */

#ifndef XUPLOADHANDLE_H
#define XUPLOADHANDLE_H

#include "XServiceHandle.h"

class XUploadHandle : public XServiceHandle
{
public:
    XUploadHandle();
    ~XUploadHandle() override;

public:
    static auto regMsgCallback() -> void;

    auto uploadFileReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto sendSliceReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto uploadFileEndReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XUPLOADHANDLE_H
