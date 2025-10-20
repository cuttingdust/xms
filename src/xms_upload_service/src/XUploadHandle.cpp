#include "XUploadHandle.h"

#include <XTools.h>
#include <XDiskCom.pb.h>

#include <fstream>

#ifdef _WIN32
#define DIR_ROOT "./server_root/"
#else
#define DIR_ROOT "/root/xms/"
#endif

class XUploadHandle::PImpl
{
public:
    PImpl(XUploadHandle *owenr);
    ~PImpl() = default;

public:
    XUploadHandle   *owenr_ = nullptr;
    xdisk::XFileInfo cur_file_;
    std::ofstream    ofs_;
};

XUploadHandle::PImpl::PImpl(XUploadHandle *owenr) : owenr_(owenr)
{
}

XUploadHandle::XUploadHandle()
{
    impl_ = std::make_unique<XUploadHandle::PImpl>(this);
}

XUploadHandle::~XUploadHandle() = default;

auto XUploadHandle::regMsgCallback() -> void
{
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_UPLOAD_FILE_REQ),
          static_cast<MsgCBFunc>(&XUploadHandle::uploadFileReq));

    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_SEND_SLICE_REQ), static_cast<MsgCBFunc>(&XUploadHandle::sendSliceReq));

    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_UPLOAD_FILE_END_REQ),
          static_cast<MsgCBFunc>(&XUploadHandle::uploadFileEndReq));
}

auto XUploadHandle::uploadFileReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    if (head->username().empty())
    {
        return;
    }

    if (!impl_->cur_file_.ParseFromArray(msg->data, msg->size))
    {
        LOGERROR("XUploadHandle::UploadFileReq ParseFromArray failed!");
        return;
    }

    xmsg::XMessageRes res;
    std::string       path = DIR_ROOT;
    path += head->username();
    path += "/";
    path += impl_->cur_file_.filedir();
    XTools::NewDir(path);

    path += "/";
    path += impl_->cur_file_.filename();
    impl_->ofs_.open(path, std::ios::out | std::ios::binary);
    if (impl_->ofs_)
    {
        res.set_return_(xmsg::XMessageRes::XR_OK);
        res.set_msg("OK");
    }
    else
    {
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg("open failed!");
    }
    head->set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_UPLOAD_FILE_RES));
    sendMsg(head, &res);
}

auto XUploadHandle::sendSliceReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
}

auto XUploadHandle::uploadFileEndReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
}
