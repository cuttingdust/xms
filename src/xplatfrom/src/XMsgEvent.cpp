#include "XMsgEvent.h"
#include "XMsgCom.pb.h"

#include <event2/bufferevent.h>

class XMsgEvent::PImpl
{
public:
    PImpl(XMsgEvent *owenr);
    ~PImpl() = default;

public:
    XMsgEvent *owenr_ = nullptr;
    XMsg       head_; /// ��Ϣͷ
    XMsg       msg_;  /// ��Ϣ����
};

XMsgEvent::PImpl::PImpl(XMsgEvent *owenr) : owenr_(owenr)
{
}


XMsgEvent::XMsgEvent()
{
    impl_ = std::make_shared<PImpl>(this);
}

XMsgEvent::~XMsgEvent() = default;

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

        /// ������ͷ�����ݽ������
        /// �����л�
        xmsg::XMsgHead pb_head;
        if (!pb_head.ParseFromArray(impl_->head_.data, impl_->head_.size))
        {
            std::cerr << "pb_head.ParseFromArray failed!" << std::endl;
            return false;
        }

        /// ��Ȩ
        /// ��Ϣ���ݴ�С
        /// ������Ϣ���ݿռ�
        if (!impl_->msg_.alloc(pb_head.msgsize()))
        {
            std::cerr << "msg_.alloc failed!" << std::endl;
            return false;
        }
        /// ������Ϣ����
        impl_->msg_.type = pb_head.msgtype();
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

void XMsgEvent::sendMsg(const xmsg::MsgType &msgType, const google::protobuf::Message *msg)
{
    if (!msg)
        return;

    xmsg::XMsgHead head;
    head.set_msgtype(msgType);

    ////////////////////////���////////////////////////

    /// ��Ϣ�������л�
    std::string msgStr  = msg->SerializeAsString();
    int         msgSize = msgStr.size();
    head.set_msgsize(msgSize);

    /// ��Ϣͷ���л�
    std::string headStr  = head.SerializeAsString();
    int         headSize = headStr.size();

    /// 1 ������Ϣͷ��С 4�ֽ� ��ʱ�������ֽ�������
    write(&headSize, sizeof(headSize));

    /// 2 ������Ϣͷ��pb���л��� XMsgHead ��������Ϣ���ݵĴ�С��
    write(headStr.data(), headStr.size());

    /// 3 ������Ϣ���� ��pb���л��� ҵ��proto
    write(msgStr.data(), msgStr.size());
}

void XMsgEvent::clear()
{
    impl_->head_.clear();
    impl_->msg_.clear();
}
