#include "XMsgEvent.h"
#include "XTools.h"

#include <XMsgCom.pb.h>

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
    XMsg            head_;              /// ��Ϣͷ
    XMsg            msg_;               /// ��Ϣ����
    xmsg::XMsgHead *pb_head_ = nullptr; /// <pb��Ϣͷ
};

XMsgEvent::PImpl::PImpl(XMsgEvent *owenr) : owenr_(owenr)
{
}


XMsgEvent::XMsgEvent()
{
    impl_ = std::make_shared<PImpl>(this);
}

XMsgEvent::~XMsgEvent() = default;

void XMsgEvent::readCB()
{
    if (!recvMsg())
    {
        std::cerr << "recvMsg failed!" << std::endl;
        clear();
        return;
    }

    auto *msg = getMsg();
    if (!msg)
    {
        std::cerr << "getMsg failed!" << std::endl;
        return;
    }

    // std::cout << "service_name = " << impl_->pb_head_->servername() << std::endl;
    LOGDEBUG(impl_->pb_head_->servername());
    readCB(impl_->pb_head_, msg);
    clear();
}

void XMsgEvent::readCB(xmsg::XMsgHead *head, XMsg *msg)
{
    /// �ص���Ϣ����
    auto iter = msg_callbacks.find(head->msgtype());
    if (iter == msg_callbacks.end())
    {
        clear();
        LOGDEBUG("msg error func not set!");
        return;
    }
    auto func = iter->second;
    (this->*func)(impl_->pb_head_, msg);
}
void XMsgEvent::regCB(const xmsg::MsgType &type, MsgCBFunc func)
{
    if (msg_callbacks.find(type) != msg_callbacks.end())
    {
        std::stringstream ss;
        ss << "regCB is error," << type << " have been set " << std::endl;
        LOGERROR(ss.str().c_str());
        return;
    }

    msg_callbacks[type] = func;
}

bool XMsgEvent::recvMsg()
{
    //////////////////////////////���/////////////////////////////

    /// ������Ϣͷ
    if (!impl_->head_.size)
    {
        /// 1 ��Ϣͷ��С
        int len = read(&impl_->head_.size, sizeof(impl_->head_.size));
        if (len <= 0 || impl_->head_.size <= 0)
        {
            return false;
        }

        /// ������Ϣͷ�ռ� ��ȡ��Ϣͷ����Ȩ����Ϣ��С��
        if (!impl_->head_.alloc(impl_->head_.size))
        {
            std::cerr << "head_.alloc failed!" << std::endl;
            return false;
        }
    }

    /// 2 ��ʼ������Ϣͷ����Ȩ����Ϣ��С��
    if (!impl_->head_.recved())
    {
        int len = read(impl_->head_.data + impl_->head_.recvSize, /// �ڶ��ν��� ���ϴε�λ�ÿ�ʼ��
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

        /// ������ͷ�����ݽ������
        /// �����л�
        if (!impl_->pb_head_->ParseFromArray(impl_->head_.data, impl_->head_.size))
        {
            std::cerr << "pb_head_.ParseFromArray failed!" << std::endl;
            return false;
        }

        /// ��Ȩ
        /// ��Ϣ���ݴ�С
        /// ������Ϣ���ݿռ�
        if (!impl_->msg_.alloc(impl_->pb_head_->msgsize()))
        {
            std::cerr << "msg_.alloc failed!" << std::endl;
            return false;
        }
        /// ������Ϣ����
        impl_->msg_.type = impl_->pb_head_->msgtype();
    }

    /// 3 ��ʼ������Ϣ����
    if (!impl_->msg_.recved())
    {
        int len = read(impl_->msg_.data + impl_->msg_.recvSize, /// �ڶ��ν��� ���ϴε�λ�ÿ�ʼ��
                       impl_->msg_.size - impl_->msg_.recvSize);
        if (len <= 0)
        {
            return true;
        }
        impl_->msg_.recvSize += len;
        if (!impl_->msg_.recved())
            return true;
    }

    if (impl_->msg_.recved())
    {
        std::cout << "msg_.recved()" << std::endl;
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
        return false;
    head->set_msgsize(msg->size);

    /// ��Ϣͷ���л�
    std::string headStr  = head->SerializeAsString();
    int         headSize = headStr.size();

    /// 1 ������Ϣͷ��С 4�ֽ� ��ʱ�������ֽ�������
    int re = write(&headSize, sizeof(headSize));
    if (!re)
        return false;

    /// 2 ������Ϣͷ��pb���л��� XMsgHead ��������Ϣ���ݵĴ�С��
    re = write(headStr.data(), headStr.size());
    if (!re)
        return false;

    /// 3 ������Ϣ���� ��pb���л��� ҵ��proto
    re = write(msg->data, msg->size);
    if (!re)
        return false;

    return true;
}

bool XMsgEvent::sendMsg(xmsg::XMsgHead *head, const google::protobuf::Message *msg)
{
    if (!msg || !head)
        return false;

    ////////////////////////���////////////////////////

    /// ��Ϣ�������л�
    std::string msgStr  = msg->SerializeAsString();
    int         msgSize = msgStr.size();

    XMsg xMsg;
    xMsg.data = (char *)msgStr.data();
    xMsg.size = msgSize;
    return sendMsg(head, &xMsg);
}

bool XMsgEvent::sendMsg(const xmsg::MsgType &msgType, const google::protobuf::Message *msg)
{
    if (!msg)
        return false;

    xmsg::XMsgHead head;
    head.set_msgtype(msgType);

    return sendMsg(&head, msg);
}

void XMsgEvent::clear()
{
    impl_->head_.clear();
    impl_->msg_.clear();
}

auto XMsgEvent::close() -> void
{
    clear();
    XComTask::close();
}
