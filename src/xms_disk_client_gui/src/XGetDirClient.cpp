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

    xmsg::XMsgHead head;
    head.set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_GET_DIR_REQ));
    head.set_servername(DIR_NAME);
    sendMsg(&head, &req);
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
}

auto XGetDirClient::regMsgCallback() -> void
{
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_GET_DIR_RES), static_cast<MsgCBFunc>(&XGetDirClient::getDirRes));
}
