#include "XGetDirClient.h"

#include "XDiskCom.pb.h"

#include "XFileManager.h"

class XGetDirClient::PImpl
{
public:
    PImpl(XGetDirClient *owenr);
    ~PImpl() = default;

public:
    XGetDirClient *owenr_ = nullptr;
    std::string    cur_dir_;
};

XGetDirClient::PImpl::PImpl(XGetDirClient *owenr) : owenr_(owenr)
{
    owenr_->setServiceName(DIR_NAME);
}

auto XGetDirClient::get() -> XGetDirClient *
{
    static XGetDirClient xc;
    return &xc;
}

XGetDirClient::XGetDirClient()
{
    impl_ = std::make_unique<XGetDirClient::PImpl>(this);
}

XGetDirClient::~XGetDirClient() = default;

auto XGetDirClient::getDirReq(xdisk::XGetDirReq req) -> void
{
    impl_->cur_dir_ = req.root();

    // xmsg::XMsgHead head;
    // head.set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_GET_DIR_REQ));
    // head.set_servername(DIR_NAME);
    // sendMsg(&head, &req);

    sendMsg(static_cast<xmsg::MsgType>(xdisk::XFMT_GET_DIR_REQ), &req);
}

auto XGetDirClient::getDirRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    xdisk::XFileInfoList file_list;
    if (!file_list.ParseFromArray(msg->data, msg->size))
    {
        std::cout << "XGetDirClient::GetDirRes failed!" << std::endl;
        return;
    }
    std::cout << file_list.DebugString();
    XFileManager::Instance()->RefreshData(file_list, impl_->cur_dir_);

    getDiskInfoReq();
}

auto XGetDirClient::newDirReq(std::string path) -> void
{
    xdisk::XGetDirReq req;
    req.set_root(path);
    sendMsg(static_cast<xmsg::MsgType>(xdisk::XFMT_NEW_DIR_REQ), &req);
}

auto XGetDirClient::newDirRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    xdisk::XGetDirReq req;
    req.set_root(impl_->cur_dir_);

    getDirReq(req);
}

auto XGetDirClient::deleteFileReq(const xdisk::XFileInfo &file) -> void
{
    sendMsg(static_cast<xmsg::MsgType>(xdisk::XFMT_DELETE_FILE_REQ), &file);
}

auto XGetDirClient::deleteFileRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    xdisk::XGetDirReq req;
    req.set_root(impl_->cur_dir_);
    getDirReq(req);
}

auto XGetDirClient::getDiskInfoReq() -> void
{
    xmsg::XMessageRes req;
    req.set_msg("GET");
    sendMsg(static_cast<xmsg::MsgType>(xdisk::XFMT_GET_DISK_INFO_REQ), &req);
}

auto XGetDirClient::getDiskInfoRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    xdisk::XDiskInfo res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        std::cout << "XGetDirClient::GetDiskInfoRes failed!" << std::endl;
        return;
    }
    XFileManager::Instance()->RefreshDiskInfo(res);
}

auto XGetDirClient::regMsgCallback() -> void
{
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_GET_DIR_RES), static_cast<MsgCBFunc>(&XGetDirClient::getDirRes));
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_NEW_DIR_RES), static_cast<MsgCBFunc>(&XGetDirClient::newDirRes));
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_DELETE_FILE_RES),
          static_cast<MsgCBFunc>(&XGetDirClient::deleteFileRes));
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_GET_DISK_INFO_RES),
          static_cast<MsgCBFunc>(&XGetDirClient::getDiskInfoRes));
}
