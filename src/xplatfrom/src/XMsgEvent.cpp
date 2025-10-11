#include "XMsgEvent.h"
#include "XTools.h"
#include "XMsgCom.pb.h"

#include <event2/bufferevent.h>

#include <sstream>
#include <iostream>

static std::map<xmsg::MsgType, XMsgEvent::MsgCBFunc> msg_callbacks;

class XMsgEvent::PImpl
{
public:
    PImpl(XMsgEvent *owenr);
    ~PImpl() = default;

public:
    XMsgEvent      *owenr_ = nullptr;
    XMsg            head_;              ///< 消息头
    XMsg            msg_;               ///< 消息内容
    xmsg::XMsgHead *pb_head_ = nullptr; ///< pb消息头
    bool            is_drop_ = false;
};

XMsgEvent::PImpl::PImpl(XMsgEvent *owenr) : owenr_(owenr)
{
}


XMsgEvent::XMsgEvent()
{
    impl_ = std::make_unique<PImpl>(this);
}

XMsgEvent::~XMsgEvent() = default;

void XMsgEvent::readCB()
{
    static int i = 0;
    i++;
    //cout << "<" << i << ">" << flush;
    /// TODO 如果线程退出
    while (true)
    {
        // std::cout << "#" << std::flush;
        if (!recvMsg())
        {
            // std::cerr << "recvMsg failed!" << std::endl;
            clear();
            break;
        }
        if (!impl_->pb_head_)
        {
            break;
        }

        auto *msg = getMsg();
        if (!msg)
        {
            std::cerr << "【RECV】 " << "getMsg failed!" << std::endl;
            return;
        }

        if (impl_->pb_head_)
        {
            //cout << "【MSG】" << pb_head_->service_name() << " " << msg->size << " " << msg->type << endl;
            std::stringstream ss;
            ss << "【RECV】" << getServerIP() << ":" << getServerPort() << "|"
               << XTools::XGetNameByPort(getServerPort()) << " " << getClientIP() << ":" << getClientPort() << " "
               << impl_->pb_head_->DebugString();
            //cout << ss.str() << endl;
            if (impl_->pb_head_->msgtype() != xmsg::MT_ADD_LOG_REQ)
            {
                LOGDEBUG(ss.str());
            }
        }


        // std::cout << "service_name = " << impl_->pb_head_->servername() << std::endl;
        // LOGDEBUG(impl_->pb_head_->servername());
        readCB(impl_->pb_head_, msg);
        clear();
        if (impl_->is_drop_)
        {
            setAutoDelete(true);
            close();
            return;
        }
    }
    // std::cout << "<" << 0 << ">" << std::flush;
}

void XMsgEvent::readCB(xmsg::XMsgHead *head, XMsg *msg)
{
    /// 回调消息函数
    auto ptr = msg_callbacks.find(head->msgtype());
    if (ptr == msg_callbacks.end())
    {
        clear();
        std::stringstream ss;
        ss << "msg error func not set!";
        ss << head->msgtype();
        LOGDEBUG(ss.str());
        return;
    }
    auto func = ptr->second;
    (this->*func)(impl_->pb_head_, msg);
}
void XMsgEvent::regCB(const xmsg::MsgType &type, MsgCBFunc func)
{
    if (msg_callbacks.contains(type))
    {
        std::stringstream ss;
        ss << "regCB is error," << type << " have been set " << std::endl;
        LOGERROR(ss.str());
        return;
    }

    msg_callbacks[type] = func;
}

bool XMsgEvent::recvMsg()
{
    //////////////////////////////解包/////////////////////////////

    /// 接收消息头
    if (!impl_->head_.size)
    {
        /// 1 消息头大小
        int len = read(&impl_->head_.size, sizeof(impl_->head_.size));
        if (len <= 0 || impl_->head_.size <= 0)
        {
            return false;
        }

        /// 分配消息头空间 读取消息头（鉴权，消息大小）
        if (!impl_->head_.alloc(impl_->head_.size))
        {
            std::stringstream ss;
            ss << "head_.Alloc failed!" << impl_->head_.size;
            LOGDEBUG(ss.str().c_str());
            return false;
        }
    }

    /// 2 开始接收消息头（鉴权，消息大小）
    if (!impl_->head_.recved())
    {
        int len = read(impl_->head_.data + impl_->head_.recvSize, /// 第二次进来 从上次的位置开始读
                       impl_->head_.size - impl_->head_.recvSize);
        if (len <= 0)
        {
            return true;
        }
        impl_->head_.recvSize += len;
        if (!impl_->head_.recved())
            return true;
        if (!impl_->pb_head_)
        {
            impl_->pb_head_ = new xmsg::XMsgHead();
        }

        /// 完整的头部数据接收完成
        /// 反序列化
        if (!impl_->pb_head_->ParseFromArray(impl_->head_.data, impl_->head_.size))
        {
            std::stringstream ss;
            ss << "pb_head.ParseFromArray failed!" << impl_->head_.size;

            LOGDEBUG(ss.str());
            return false;
        }

        /// 空包数据
        if (impl_->pb_head_->msgsize() == 0)
        {
            // std::cout << "0" << std::flush;
            /// 消息类型
            impl_->msg_.type = impl_->pb_head_->msgtype();
            impl_->msg_.size = 0;
            return true;
        }
        else
        {
            /// 鉴权
            /// 消息内容大小
            /// 分配消息内容空间
            if (!impl_->msg_.alloc(impl_->pb_head_->msgsize()))
            {
                std::stringstream ss;
                ss << "msg_.Alloc failed!msg_size=" << impl_->pb_head_->msgsize();
                LOGDEBUG(ss.str());
                return false;
            }
        }


        /// 设置消息类型
        impl_->msg_.type = impl_->pb_head_->msgtype();
    }

    /// 3 开始接收消息内容
    if (!impl_->msg_.recved())
    {
        int len = read(impl_->msg_.data + impl_->msg_.recvSize, /// 第二次进来 从上次的位置开始读
                       impl_->msg_.size - impl_->msg_.recvSize);
        if (len <= 0)
        {
            return true;
        }
        impl_->msg_.recvSize += len;
    }

    if (impl_->msg_.recved())
    {
        std::cout << "+" << std::flush;
    }

    return true;
}

auto XMsgEvent::getMsg() const -> XMsg *
{
    return impl_->msg_.recved() ? &impl_->msg_ : nullptr;
}

auto XMsgEvent::sendMsg(xmsg::XMsgHead *head, XMsg *msg) -> bool
{
    if (!head || !msg)
    {
        return false;
    }

    head->set_msgsize(msg->size);

    /// 消息头序列化
    std::string headStr  = head->SerializeAsString();
    int         headSize = headStr.size();

    if (head)
    {
        std::stringstream ss;
        ss << "【SEND】" << getServerIP() << ":" << getServerPort() << " " << XTools::XGetNameByPort(getServerPort())
           << " " << head->DebugString();
        //cout << ss.str() << endl;
        // 在记录日志会死循环
        if (head->msgtype() != xmsg::MT_ADD_LOG_REQ)
            LOGDEBUG(ss.str());
    }

    /// 1 发送消息头大小 4字节 暂时不考虑字节序问题
    int re = write(&headSize, sizeof(headSize));
    if (!re)
    {
        return false;
    }

    /// 2 发送消息头（pb序列化） XMsgHead （设置消息内容的大小）
    re = write(headStr.data(), headStr.size());
    if (!re)
    {
        return false;
    }

    /// 支持发送空包
    if (msg->size > 0)
    {
        re = write(msg->data, msg->size);
        if (!re)
        {
            return false;
        }
    }
    return true;
}

bool XMsgEvent::sendMsg(xmsg::XMsgHead *head, const google::protobuf::Message *msg)
{
    if (!msg || !head)
        return false;

    ////////////////////////封包////////////////////////

    /// 消息内容序列化
    std::string msgStr  = msg->SerializeAsString();
    int         msgSize = msgStr.size();

    XMsg xMsg;
    xMsg.data = (char *)msgStr.data();
    xMsg.size = msgSize;
    return sendMsg(head, &xMsg);
}

auto XMsgEvent::sendMsg(const xmsg::MsgType &msgType, const google::protobuf::Message *msg) -> bool
{
    if (!msg)
        return false;

    xmsg::XMsgHead head;
    head.set_msgtype(msgType);

    return sendMsg(&head, msg);
}

auto XMsgEvent::clear() -> void
{
    impl_->head_.clear();
    impl_->msg_.clear();
}

auto XMsgEvent::close() -> void
{
    clear();
    XComTask::close();
}
