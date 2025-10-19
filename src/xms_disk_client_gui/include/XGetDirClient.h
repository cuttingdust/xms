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

/// 定时从任务列表获取任务
/// 需要考虑登录，是否放在XServiceClient中
/// 要加入鉴权的内容

namespace xdisk
{
    class XFileInfo;
    class XGetDirReq;
} // namespace xdisk

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

    auto newDirReq(std::string path) -> void;

    auto newDirRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto deleteFileReq(const xdisk::XFileInfo &file) -> void;

    auto deleteFileRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto getDiskInfoReq() -> void;

    auto getDiskInfoRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    static auto regMsgCallback() -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XGETDIRCLIENT_H
