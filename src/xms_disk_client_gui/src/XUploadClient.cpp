#include "XUploadClient.h"

#include "XFileManager.h"

#include <XTools.h>
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
    XUploadClient         *owenr_ = nullptr;
    xdisk::XFileInfo       file_;                    ///< 上传文件信息
    std::ifstream          ifs_;                     ///< 上传的文件
    char                  *slice_buf_     = nullptr; ///< 读取文件片是用的缓存
    char                  *slice_buf_enc_ = nullptr; ///< 读取文件片是用的缓存(加密后)
    std::list<std::string> md5_base64s_;
    long long              begin_send_data_size_ = -1; ///< 开始发送数据时，已经发送的值，要确保缓冲已经都发送成功

    XAES       *aes_ = nullptr;
    std::string password_;
    std::mutex  password_mutex_;
    int         task_id_ = 0;
};

XUploadClient::PImpl::PImpl(XUploadClient *owenr) : owenr_(owenr)
{
    slice_buf_ = new char[FILE_SLICE_BYTE];

    /// 不是16的倍数要补全，所以要多预留空间
    slice_buf_enc_ = new char[FILE_SLICE_BYTE + 16];

    /// 通过定时器跟踪进度
    owenr_->setTimeMs(100);
}

XUploadClient::PImpl::~PImpl()
{
    delete slice_buf_;
    slice_buf_ = nullptr;

    delete slice_buf_enc_;
    slice_buf_enc_ = nullptr;
    if (aes_)
    {
        aes_->Drop();
        aes_ = nullptr;
    }
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
    head.set_offset(offset);

    ifs_.read(slice_buf_, FILE_SLICE_BYTE);
    size = ifs_.gcount();

    XMsg data;
    data.data = slice_buf_;
    data.size = size;

    /// 如果需要加密
    if (file_.is_enc())
    {
        long long enc_size = 0;

        if (!aes_)
        {
            LOGERROR("aes not init!");
            return;
        }
        enc_size               = aes_->Encrypt(reinterpret_cast<unsigned char *>(slice_buf_), size,
                                               reinterpret_cast<unsigned char *>(slice_buf_enc_));
        data.data              = slice_buf_enc_;
        data.size              = enc_size;
        std::string md5_base64 = XTools::XMD5_base64(reinterpret_cast<unsigned char *>(slice_buf_enc_), enc_size);
        head.set_md5(md5_base64);
    }
    else
    {
        /// 未加密
        if (!md5_base64s_.empty())
        {
            head.set_md5(md5_base64s_.front());
            md5_base64s_.pop_front();
        }
    }

    xdisk::XFileInfo *info = new xdisk::XFileInfo;
    info->CopyFrom(file_);
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
    file.clear_password(); /// 不要发送密码
    sendMsg(static_cast<xmsg::MsgType>(xdisk::XFMT_UPLOAD_FILE_REQ), &file);
    std::cout << "XUploadClient::Connect()" << std::endl;
}

auto XUploadClient::timerCB() -> void
{
    if (impl_->begin_send_data_size_ < 0)
    {
        return;
    }

    auto sendSize  = getSendDataSize();
    auto size      = bufferSize();
    auto beginSize = impl_->begin_send_data_size_;

    /// 已发送的数据 不完全准确，还有确认的数据包发送
    long long sended = sendSize - beginSize - size;

    std::cout << sended << ":" << impl_->file_.filesize() << std::endl;

    /// 如果数据过大，先缩小
    XFileManager::Instance()->uploadProcess(impl_->task_id_, sended);
}

auto XUploadClient::uploadFileRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    std::cout << "UploadFileRes 1 " << std::endl;
    /// 开始发送数据时，已经发送的值，要确保缓冲已经都发送成功
    /// 根据协议，接收到服务器的反馈，缓冲肯定已发送完毕
    impl_->begin_send_data_size_ = this->getSendDataSize();
    /// 开始发送文件
    impl_->sendSlice();
}

auto XUploadClient::uploadFileEndRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    std::cout << "UploadFileEndRes 3" << std::endl;
    XFileManager::Instance()->uploadEnd(impl_->task_id_);
    XFileManager::Instance()->refreshDir();

    /// 任务完成刷新界面
    clearTimer();
    close();
    dropInMsg();
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

    /// 获取md5 每个FILE_SLICE_BYTE 生成一个片md5
    /// 所有的片md5，再生成一个文件md5
    /// 一开始就生成md5的目的是为了后面的秒传
    int         filesize       = 0;
    char        md5_base64[17] = { 0 };
    std::string all_md5_base64 = "";
    while (!impl_->ifs_.eof())
    {
        impl_->ifs_.read(impl_->slice_buf_, FILE_SLICE_BYTE);
        int size = impl_->ifs_.gcount();
        filesize += size;

        std::string md5_base64 = XTools::XMD5_base64(reinterpret_cast<unsigned char *>(impl_->slice_buf_), size);
        impl_->md5_base64s_.push_back(md5_base64);
        all_md5_base64 += md5_base64;
        //cout << "[" << md5_base64 << "]" << flush;
    }

    if (filesize == 0)
    {
        impl_->ifs_.close();
        return false;
    }

    /// 生成文件的md5 解密后校验，普通文件一开始就生成，加密文件不考虑秒传
    std::string file_md5 =
            XTools::XMD5_base64(reinterpret_cast<unsigned char *>(all_md5_base64.data()), all_md5_base64.size());
    impl_->file_.set_md5(file_md5);

    impl_->file_.set_filesize(filesize);

    /// 如果是加密文件 文件大小补齐16的倍数
    if (impl_->file_.is_enc())
    {
        int dec_size = 0;
        if (filesize % 16 != 0)
        {
            dec_size = filesize + (16 - filesize % 16);
        }
        impl_->file_.set_filesize(dec_size);
        impl_->file_.set_ori_size(filesize);
    }

    impl_->ifs_.clear();
    impl_->ifs_.seekg(0, std::ios_base::beg);
    std::cout << impl_->file_.DebugString();

    if (impl_->file_.is_enc())
    {
        auto pass = impl_->file_.password();
        if (pass.empty())
        {
            LOGERROR("please set  password");
            return false;
        }
        if (!impl_->aes_)
        {
            impl_->aes_ = XAES::Create();
        }
        impl_->aes_->SetKey(pass.c_str(), pass.size(), true);
    }
    return true;
}

auto XUploadClient::set_task_id(int task_id) -> void
{
    impl_->task_id_ = task_id;
}

auto XUploadClient::task_id() const -> int
{
    return impl_->task_id_;
}
