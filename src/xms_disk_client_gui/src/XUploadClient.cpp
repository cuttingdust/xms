#include "XUploadClient.h"

#include <XDiskCom.pb.h>

#include <fstream>

class XUploadClient::PImpl
{
public:
    PImpl(XUploadClient *owenr);
    ~PImpl() = default;

public:
    XUploadClient   *owenr_ = nullptr;
    xdisk::XFileInfo file_; ///< 上传文件信息
    std::ifstream    ifs_;  ///< 上传的文件
};

XUploadClient::PImpl::PImpl(XUploadClient *owenr) : owenr_(owenr)
{
}

XUploadClient::XUploadClient()
{
    impl_ = std::make_unique<XUploadClient::PImpl>(this);
}

XUploadClient::~XUploadClient()
{
}

auto XUploadClient::connectCB() -> void
{
    auto file = impl_->file_;

    sendMsg(static_cast<xmsg::MsgType>(xdisk::XFMT_UPLOAD_FILE_REQ), &file);
    std::cout << "XUploadClient::Connect()" << std::endl;
}

auto XUploadClient::uploadFileRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    std::cout << "UploadFileRes 1 " << std::endl;
}

auto XUploadClient::uploadFileEndRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
}

auto XUploadClient::sendSliceRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
}

auto XUploadClient::regMsgCallback() -> void
{
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_UPLOAD_FILE_RES),
          static_cast<MsgCBFunc>(&XUploadClient::uploadFileRes));
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_UPLOAD_FILE_END_RES),
          static_cast<MsgCBFunc>(&XUploadClient::uploadFileEndRes));
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_SEND_SLICE_RES), static_cast<MsgCBFunc>(&XUploadClient::sendSliceRes));
}

auto XUploadClient::setFile(const xdisk::XFileInfo &file) -> bool
{
    impl_->file_ = file;
    impl_->ifs_.open(impl_->file_.local_path(), std::ios::in | std::ios::binary);
    if (!impl_->ifs_.is_open())
    {
        return false;
    }

    int filesize = 0;
}
