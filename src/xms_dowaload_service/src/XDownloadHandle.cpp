#include "XDownloadHandle.h"

#include <XDiskCom.pb.h>
#include <XTools.h>

#include <algorithm>
#include <fstream>

#ifdef _WIN32
#define DIR_ROOT "./server_root/"
#else
#define DIR_ROOT "/mnt/xms/"
#endif
#define FILE_INFO_NAME_PRE ".info_"
#define FILE_SLICE_BYTE    100000000

class XDownloadHandle::PImpl
{
public:
    PImpl(XDownloadHandle *owenr);
    ~PImpl();

public:
    auto sendSlice() -> void;

public:
    XDownloadHandle  *owenr_ = nullptr;
    xdisk::XFileInfo  file_;          ///< 当前接收的文件
    xdisk::XFileSlice cur_slice_;     ///< 当前接收的文件片信息
    std::list<XMsg>   cur_data_;      ///< 当前接收的文件片数据
    std::list<XMsg>   caches_;        ///< 接收文件的缓存
    std::ifstream     ifs_;           ///< 写入本地文件
    int               filesize_  = 0; ///< 文件大小
    int               sendsize_  = 0; ///< 已经发送的文件大小
    char             *slice_buf_ = 0;
};

XDownloadHandle::PImpl::PImpl(XDownloadHandle *owenr) : owenr_(owenr)
{
    /// 设定定时器用于获取传送进度
    owenr_->setTimeMs(100);
    slice_buf_ = new char[FILE_SLICE_BYTE];
}

XDownloadHandle::PImpl::~PImpl()
{
    delete slice_buf_;
    slice_buf_ = nullptr;
}

auto XDownloadHandle::PImpl::sendSlice() -> void
{
    if (ifs_.eof())
    {
        /// UPLOAD_FILE_END_REQ
        return;
    }

    int size = FILE_SLICE_BYTE;
    size     = std::min<int64_t>(size, file_.filesize());

    ifs_.read(slice_buf_, FILE_SLICE_BYTE);
    size = ifs_.gcount();

    xdisk::XFileInfo *info = new xdisk::XFileInfo();
    info->CopyFrom(file_);
    xmsg::XMsgHead head;
    head.set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_DOWNLOAD_SLICE_REQ));
    XMsg data;
    data.data = slice_buf_;
    data.size = size;
    owenr_->sendMsg(&head, &data);
}

XDownloadHandle::XDownloadHandle()
{
    impl_ = std::make_unique<XDownloadHandle::PImpl>(this);
}

XDownloadHandle::~XDownloadHandle() = default;

auto XDownloadHandle::regMsgCallback() -> void
{
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_DOWNLOAD_FILE_REQ),
          static_cast<MsgCBFunc>(&XDownloadHandle::downloadFileReq));

    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_DOWNLOAD_FILE_BEGTIN),
          static_cast<MsgCBFunc>(&XDownloadHandle::downloadFileBegin));

    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_DOWNLOAD_SLICE_RES),
          static_cast<MsgCBFunc>(&XDownloadHandle::downloadSliceRes));
}

auto XDownloadHandle::downloadFileReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    /// 验证用户权限
    /// 接收到文件请求
    if (!impl_->file_.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("UploadFileReq ParseFromArray failed!");
        return;
    }

    /// DOWNLOAD_FILE_RES
    /// 容错没有信息的情况，有客户端判断文件是否有效


    xmsg::XMessageRes res;
    std::string       path = DIR_ROOT;
    path += head->username();
    path += "/";
    path += impl_->file_.filedir();
    path += "/";
    std::string filedir = path;
    path += impl_->file_.filename();

    std::string info_file = filedir;

    info_file += FILE_INFO_NAME_PRE;
    info_file += impl_->file_.filename();
    xdisk::XFileInfo re_file;
    std::ifstream    ifs(info_file);
    if (!ifs || !re_file.ParseFromIstream(&ifs))
    {
        LOGINFO("file info read failed");
        re_file.CopyFrom(impl_->file_);
    }
    ifs.close();


    head->set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_DOWNLOAD_FILE_RES));
    impl_->ifs_.open(path, std::ios::in | std::ios::binary);
    impl_->ifs_.seekg(0, std::ios::end);
    if (!impl_->ifs_)
    {
        /// 失败返回文件大小为0
        re_file.set_filesize(0);
        sendMsg(head, &re_file);
        return;
    }
    re_file.set_filesize(impl_->ifs_.tellg());
    impl_->ifs_.seekg(0, std::ios::beg);
    sendMsg(head, &re_file);
}

auto XDownloadHandle::downloadFileBegin(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    impl_->sendSlice();
}

auto XDownloadHandle::downloadSliceRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    impl_->sendSlice();
}
