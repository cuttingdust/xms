/**
 * @file   XGetDirClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-13
 */

#ifndef XGETDIRCLIENT_H
#define XGETDIRCLIENT_H

namespace xdisk
{
    class XGetDirReq;
}

#include <XServiceClient.h>

class XGetDirClient : public XServiceClient
{
public:
    static auto get() -> XGetDirClient *;

private:
    XGetDirClient();
    ~XGetDirClient() override;

public:
    auto getDirReq(xdisk::XGetDirReq req) -> void;

    auto getDirRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    static auto regMsgCallback() -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XGETDIRCLIENT_H
