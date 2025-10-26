#include "XDownloadClient.h"

#include "XFileManager.h"

#include <XTools.h>
#include <XDiskCom.pb.h>

#include <fstream>


class XDownloadClient::PImpl
{
public:
    PImpl(XDownloadClient *owenr);
    ~PImpl();

public:
public:
    XDownloadClient *owenr_ = nullptr;


    xdisk::XFileInfo file_;
    std::ofstream    ofs_;
    XAES            *aes_                  = nullptr; ///< 加密文件用
    int              task_id_              = 0;
    std::string      all_md5_base64_       = "";
    long long        begin_recv_data_size_ = -1; ///< 开始发送数据时，已经发送的值，要确保缓冲已经都发送成功
};

XDownloadClient::PImpl::PImpl(XDownloadClient *owenr) : owenr_(owenr)
{
    owenr_->setTimeMs(100);
}

XDownloadClient::PImpl::~PImpl()
{
    if (aes_)
    {
        aes_->Drop();
        aes_ = nullptr;
    }
}

XDownloadClient::XDownloadClient()
{
    impl_ = std::make_unique<XDownloadClient::PImpl>(this);
}

XDownloadClient::~XDownloadClient()
{
}

auto XDownloadClient::connectCB() -> void
{
    sendMsg(static_cast<xmsg::MsgType>(xdisk::XFMT_DOWNLOAD_FILE_REQ), &impl_->file_);
    std::cout << "XDownloadClient::Connect()" << std::endl;
}

auto XDownloadClient::timerCB() -> void
{
    if (impl_->begin_recv_data_size_ < 0)
    {
        return;
    }

    auto size = bufferSize();

    /// 已发送的数据
    long long recved = getRecvDataSize() - impl_->begin_recv_data_size_;

    std::cout << recved << ":" << impl_->file_.filesize() << std::endl;
    XFileManager::Instance()->downloadProcess(impl_->task_id_, recved);
}

auto XDownloadClient::set_task_id(int task_id) -> void
{
    impl_->task_id_ = task_id;
}

auto XDownloadClient::task_id() const -> int
{
    return impl_->task_id_;
}

auto XDownloadClient::setFile(const xdisk::XFileInfo &file) -> bool
{
    impl_->file_ = file;
    impl_->ofs_.open(file.local_path(), std::ios::binary);
    if (!impl_->ofs_.is_open())
    {
        std::cout << "set_file " << file.local_path() << std::endl;
        return false;
    }
    return true;
}

auto XDownloadClient::downloadFileRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    /// 确认文件信息
    if (!impl_->file_.ParseFromArray(msg->data, msg->size))
    {
        std::cout << "XDownloadClient::DownloadFileRes ParseFromArray failed!" << std::endl;
        return;
    }
    /// 文件加密，需要有秘钥
    if (impl_->file_.is_enc())
    {
        auto pass = XFileManager::Instance()->password();
        if (pass.empty())
        {
            LOGERROR("please set password");
            /// 具体的提示的语言，可以根据字符串替换为不同的语言
            XFileManager::Instance()->ErrorSig("NO PASSWORD");
            return;
        }
        impl_->aes_ = XAES::Create();
        impl_->aes_->SetKey(pass.c_str(), pass.size(), false);
    }

    /// 如果是加密文件需要验证加密
    int task_id     = XFileManager::Instance()->addDownloadTask(impl_->file_);
    impl_->task_id_ = task_id;


    sendMsg(static_cast<xmsg::MsgType>(xdisk::XFMT_DOWNLOAD_FILE_BEGTIN), &impl_->file_);

    impl_->begin_recv_data_size_ = getRecvDataSize();
    XFileManager::Instance()->downloadProcess(task_id, 0);
}

auto XDownloadClient::downloadSliceReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    int recved = impl_->file_.net_size() + msg->size;
    impl_->file_.set_net_size(recved);
    const char *data = msg->data;
    int         size = msg->size;
    if (impl_->file_.is_enc())
    {
        char *dec_data = new char[msg->size];
        size           = impl_->aes_->Decrypt(reinterpret_cast<unsigned char *>(msg->data), msg->size,
                                              reinterpret_cast<unsigned char *>(dec_data));
        if (size <= 0)
        {
            LOGERROR("aes_->Decrypt failed!");
            delete dec_data;
            return;
        }
        if (recved > impl_->file_.ori_size())
        {
            size = size - (recved - impl_->file_.ori_size());
        }
        data = dec_data;
    }

    std::string md5_base64 = XTools::XMD5_base64((unsigned char *)data, size);
    // md5_base64s_.push_back(md5_base64);
    impl_->all_md5_base64_ += md5_base64;

    impl_->ofs_.write(data, size);
    if (impl_->file_.is_enc())
    {
        delete data;
    }

    sendMsg(static_cast<xmsg::MsgType>(xdisk::XFMT_DOWNLOAD_SLICE_RES), &impl_->file_);
    /// 文件接收结束
    if (impl_->file_.filesize() == impl_->file_.net_size())
    {
        XFileManager::Instance()->downloadEnd(impl_->task_id_);
        /// 校验整个文件的md5
        impl_->ofs_.close();

        std::string file_md5 = XTools::XMD5_base64(reinterpret_cast<unsigned char *>(impl_->all_md5_base64_.data()),
                                                   impl_->all_md5_base64_.size());
        if (impl_->file_.md5() != file_md5)
        {
            std::cerr << "file is not complete" << std::endl;
        }

        clearTimer();
        close();
    }
}


auto XDownloadClient::regMsgCallback() -> void
{
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_DOWNLOAD_FILE_RES),
          static_cast<MsgCBFunc>(&XDownloadClient::downloadFileRes));

    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_DOWNLOAD_SLICE_REQ),
          static_cast<MsgCBFunc>(&XDownloadClient::downloadSliceReq));
}
