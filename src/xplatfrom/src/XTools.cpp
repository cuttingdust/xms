#include "XTools.h"

#include "XMsgType.pb.h"
#include "XMsgCom.pb.h"
#include "XMsg.h"

#include <openssl/md5.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int Base64Encode(const unsigned char *in, int len, char *out_base64)
{
    if (!in || len <= 0 || !out_base64)
    {
        return 0;
    }

    /// 内存源 source
    auto mem_bio = BIO_new(BIO_s_mem());
    if (!mem_bio)
    {
        return 0;
    }

    /// base64 filter
    auto b64_bio = BIO_new(BIO_f_base64());
    if (!b64_bio)
    {
        BIO_free(mem_bio);
        return 0;
    }

    /// 形成BIO链
    /// b64-mem
    BIO_push(b64_bio, mem_bio);

    /// 超过64字节不添加换行（\n）,编码的数据在一行中
    /// 默认结尾有换行符\n 超过64字节再添加\n
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);

    ///  写入到base64 filter 进行编码，结果会传递到链表的下一个节点
    /// 到mem中读取结果(链表头部代表了整个链表)
    /// BIO_write 编码 3字节=》4字节  不足3字节补充0 和 =
    /// 编码数据每64字节（不确定）会加\n 换行符
    int re = BIO_write(b64_bio, in, len);
    if (re <= 0)
    {
        /// 情况整个链表节点
        BIO_free_all(b64_bio);
        return 0;
    }

    ///刷新缓存，写入链表的mem
    BIO_flush(b64_bio);

    int outsize = 0;
    /// 从链表源内存读取
    BUF_MEM *p_data = NULL;
    BIO_get_mem_ptr(b64_bio, &p_data);
    if (p_data)
    {
        memcpy(out_base64, p_data->data, p_data->length);
        outsize = p_data->length;
    }
    BIO_free_all(b64_bio);
    return outsize;
}

int Base64Decode(const char *in, int len, unsigned char *out_data)
{
    if (!in || len <= 0 || !out_data)
    {
        return 0;
    }

    /// 内存源 （密文）
    auto mem_bio = BIO_new_mem_buf(in, len);
    if (!mem_bio)
    {
        return 0;
    }

    /// base64 过滤器
    auto b64_bio = BIO_new(BIO_f_base64());
    if (!b64_bio)
    {
        BIO_free(mem_bio);
        return 0;
    }

    /// 形成BIO链
    BIO_push(b64_bio, mem_bio);

    /// 默认读取换行符做结束
    /// 设置后编码中如果有\n会失败
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);

    /// 读取 解码 4字节转3字节
    size_t size = 0;
    BIO_read_ex(b64_bio, out_data, len, &size);
    BIO_free_all(b64_bio);
    return size;
}

/// 生成md5 128bit(16字节)
unsigned char *XMD5(const unsigned char *d, unsigned long n, unsigned char *md)
{
    return MD5(d, n, md);
}

/////////////////////////////////////////////////////////////////////////////////////////////////


std::string XTools::GetDirData(std::string path)
{
    std::string data = "";
    try
    {
        for (const auto &entry : fs::directory_iterator(path))
        {
            std::string tmp = "";
            if (entry.is_directory())
            {
                continue;
            }

            const auto &file     = entry.path();
            const auto &fileName = file.filename().string();
            const auto &fileSize = fs::file_size(file);

            tmp = std::format("{},{} Byte;", fileName, fileSize);

            data += tmp;
        }
        if (!data.empty())
        {
            data = data.substr(0, data.size() - 1);
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Error while listing directory: " << e.what() << std::endl;
        return "";
    }
    catch (...)
    {
        std::cerr << "Unknown error occurred." << std::endl;
        return "";
    }
    return data;
}

std::string XTools::XMD5_base64(const unsigned char *d, unsigned long n)
{
    unsigned char buf[16] = { 0 };
    XMD5(d, n, buf);
    char base64[32] = { 0 };
    Base64Encode(buf, 16, base64);
    return base64;
}

char *XTools::XMD5_base64(const unsigned char *d, unsigned long n, char *md)
{
    unsigned char buf[16] = { 0 };
    XMD5(d, n, buf);
    Base64Encode(buf, 16, md);
    return md;
}

auto XTools::XGetTime(int timestamp, std::string fmt) -> std::string
{
    char   time_buf[128] = { 0 };
    time_t tm            = timestamp;
    if (timestamp <= 0)
        tm = time(0);
    strftime(time_buf, sizeof(time_buf), fmt.c_str(), gmtime(&tm));
    return time_buf;
}

auto XTools::XGetPortName(unsigned short port) -> const char *
{
    switch (port)
    {
        case API_GATEWAY_PORT:
            return API_GATEWAY_NAME;
            break;
        case REGISTER_PORT:
            return REGISTER_NAME;
            break;
        case CONFIG_PORT:
            return AUTH_NAME;
            break;
        case XLOG_PORT:
            return XLOG_NAME;
            break;
        case DOWNLOAD_PORT:
            return DOWNLOAD_NAME;
            break;
        case DIR_PORT:
            return DIR_NAME;
            break;
        case UPLOAD_PORT:
            return UPLOAD_NAME;
            break;
        default:
            break;
    }
    return "";
}

auto XTools::PrintMsg(xmsg::XMsgHead *head, XMsg *msg)
{
    std::stringstream ss;
    ss << "【MSG】";
    if (head)
    {
        ss << head->servername();
    }
    std::cout << "【MSG】" << head->servername() << " " << msg->size << " " << msg->type << std::endl;
    if (msg)
    {
        google::protobuf::int32 msg_size = 1;

        /// 消息类型
        xmsg::MsgType msg_type = xmsg::MT_LOGIN_RES;

        /// 令牌 如果时登陆消息则未空
        std::string token = std::to_string(3);

        /// 微服务的名称，用于路由
        std::string service_name = std::to_string(4);
    }
}

XMutex::XMutex(std::mutex *mux)
{
    mux_ = mux;
    mux_->lock();
}

XMutex::~XMutex()
{
    mux_->unlock();
}
