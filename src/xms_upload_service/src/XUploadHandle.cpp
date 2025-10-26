#include "XUploadHandle.h"

#include <XTools.h>
#include <XDiskCom.pb.h>

#include <fstream>

#ifdef _WIN32
#define DIR_ROOT "./server_root/"
#else
#define DIR_ROOT "/mnt/xms/"
#endif
#define FILE_INFO_NAME_PRE ".info_"
class XUploadHandle::PImpl
{
public:
    PImpl(XUploadHandle *owenr);
    ~PImpl();

public:
    XUploadHandle    *owenr_    = nullptr;
    std::string       save_dir_ = "";
    xdisk::XFileInfo  cur_file_;     ///< 当前接收的文件
    xdisk::XFileSlice cur_slice_;    ///< 当前接收的文件片信息
    std::list<XMsg>   cur_data_;     ///< 当前接收的文件片数据
    std::list<XMsg>   caches_;       ///< 接收文件的缓存
    std::ofstream     ofs_;          ///< 写入本地文件
    int               filesize_ = 0; ///< 文件大小
    int               sendsize_ = 0; ///< 已经发送的文件大小
    XAES             *aes_      = 0; ///< 加密文件
};

XUploadHandle::PImpl::PImpl(XUploadHandle *owenr) : owenr_(owenr)
{
    /// 设定定时器用于获取传送进度
    owenr_->setTimeMs(100);
}

XUploadHandle::PImpl::~PImpl()
{
    delete aes_;
    aes_ = nullptr;
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
    /// 验证用户权限
    /// 接收到文件请求
    if (!impl_->cur_file_.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("UploadFileReq ParseFromArray failed!");
        return;
    }

    xmsg::XMessageRes res;
    std::string       path = DIR_ROOT;
    path += head->username();
    path += "/";
    path += impl_->cur_file_.filedir();
    impl_->save_dir_ = path;
    path += "/";
    std::cout << "UploadFileReq path = " << path << std::endl;
    /// 创建目录
    XTools::NewDir(path);

    path += impl_->cur_file_.filename();

    impl_->ofs_.open(path, std::ios::out | std::ios::binary);

    /// 需要校验权限
    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK");
    if (!impl_->ofs_.is_open())
    {
        std::stringstream ss;
        ss << "UploadFileReq open file failed!" << path;
        res.set_return_(xmsg::XMessageRes::XR_ERROR);

        res.set_msg(ss.str());
        LOGINFO(ss.str());
    }

    /// 目录修改位实际目录 需要添加公共配置 上传目录

    // res.set_return_(XMessageRes::ERROR);

    head->set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_UPLOAD_FILE_RES));
    sendMsg(head, &res);
}

auto XUploadHandle::sendSliceReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    head->set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_SEND_SLICE_RES));
    xmsg::XMessageRes res;
    if (head->md5().empty())
    {
        std::cout << "XUploadHandle::SendSliceReq failed! md5 is empty!";

        /// 需要校验权限
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg("md5 is empty");
        sendMsg(head, &res);
        return;
    }
    /// 校验md5
    std::string md5 = XTools::XMD5_base64(reinterpret_cast<unsigned char *>(msg->data), msg->size);
    if (head->md5() != md5)
    {
        std::cout << "XUploadHandle::SendSliceReq failed! md5 is error!";
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg("md5 is error");
        return;
    }

    impl_->ofs_.write(msg->data, msg->size);

    /// 需要校验权限
    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK");
    sendMsg(head, &res);
}

auto XUploadHandle::uploadFileEndReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    impl_->ofs_.close();
    /// 文件信息存储 .filename.info
    std::string info_path = impl_->save_dir_;
    info_path += FILE_INFO_NAME_PRE;
    info_path += impl_->cur_file_.filename();
    std::ofstream ofs;
    ofs.open(info_path, std::ios::out | std::ios::binary);
    if (ofs)
    {
        impl_->cur_file_.SerializeToOstream(&ofs);
        ofs.close();
    }

    /// 验证文件md5 验证是否正确
    head->set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_UPLOAD_FILE_END_RES));
    xmsg::XMessageRes res;
    /// 需要校验权限
    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK");
    sendMsg(head, &res);
}
