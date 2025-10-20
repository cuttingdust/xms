#include "XUploadClient.h"

#include <XDiskCom.pb.h>

#include <algorithm>
#include <fstream>


/// 接近100M每个任务缓存 100m 任务过多时，开销大
/// 每个100兆消息等待回应，至少消耗100ms
/// 暂时不考虑 速度，后面要考虑做一个数据块列表
#define FILE_SLICE_BYTE 10000000
//#define FILE_SLICE_BYTE 10000

class XUploadClient::PImpl
{
public:
    PImpl(XUploadClient *owenr);
    ~PImpl();

public:
    /// \brief 读取文件 并发送文件片
    auto sendSlice() -> void;

public:
    XUploadClient   *owenr_ = nullptr;
    xdisk::XFileInfo file_;                ///< 上传文件信息
    std::ifstream    ifs_;                 ///< 上传的文件
    char            *slice_buf_ = nullptr; ///< 读取文件片是用的缓存
};

XUploadClient::PImpl::PImpl(XUploadClient *owenr) : owenr_(owenr)
{
    slice_buf_ = new char[FILE_SLICE_BYTE];
}

XUploadClient::PImpl::~PImpl()
{
    delete slice_buf_;
}

auto XUploadClient::PImpl::sendSlice() -> void
{
    if (ifs_.eof())
    {
        owenr_->sendMsg(static_cast<xmsg::MsgType>(xdisk::XFMT_UPLOAD_FILE_END_REQ), &file_);
        ///文件到结尾，发送结束
        return;
    }

    int size = FILE_SLICE_BYTE;
    size     = std::min<int>(size, file_.filesize());
    /// 当前文件的偏移位置
    long long      offset = ifs_.tellg();
    xmsg::XMsgHead head;
    head.set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_SEND_SLICE_REQ));

    ifs_.read(slice_buf_, FILE_SLICE_BYTE);
    size = ifs_.gcount();

    XMsg data;
    data.data = slice_buf_;
    data.size = size;

    owenr_->setHead(&head);
    owenr_->sendMsg(&head, &data);
}

XUploadClient::XUploadClient()
{
    impl_ = std::make_unique<XUploadClient::PImpl>(this);
}

XUploadClient::~XUploadClient() = default;

auto XUploadClient::connectCB() -> void
{
    auto file = impl_->file_;

    sendMsg(static_cast<xmsg::MsgType>(xdisk::XFMT_UPLOAD_FILE_REQ), &file);
    std::cout << "XUploadClient::Connect()" << std::endl;
}

auto XUploadClient::uploadFileRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    std::cout << "UploadFileRes 1 " << std::endl;
    /// 开始发送文件
    impl_->sendSlice();
}

auto XUploadClient::uploadFileEndRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    std::cout << "UploadFileEndRes 3" << std::endl;
}

auto XUploadClient::sendSliceRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    std::cout << "SendSliceRes 2 " << std::endl;
    impl_->sendSlice();
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

    if (file.filesize() <= 0)
    {
        impl_->ifs_.seekg(0, std::ios::end);
        impl_->file_.set_filesize(impl_->ifs_.tellg());
        impl_->ifs_.seekg(0, std::ios::beg);
    }

    int filesize = 0;
}
