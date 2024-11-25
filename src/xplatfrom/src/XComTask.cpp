#include "XComTask.h"
#include "XMsg.h"

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <iostream>

#include <event2/bufferevent.h>

static void SReadCb(struct bufferevent *bev, void *ctx)
{
    auto task = static_cast<XComTask *>(ctx);
    task->readCB();
}

static void SWriteCb(struct bufferevent *bev, void *ctx)
{
    auto task = static_cast<XComTask *>(ctx);
    task->writeCB();
}

static void SEventCb(struct bufferevent *bev, short events, void *ctx)
{
    auto task = static_cast<XComTask *>(ctx);
    task->eventCB(events);
}

class XComTask::PImpl
{
public:
    PImpl(XComTask *owenr);
    ~PImpl();

public:
    XComTask           *owenr_        = nullptr;
    struct bufferevent *bev_          = nullptr;
    std::string         serverPath_   = "";
    std::string         serverIp_     = "";
    int                 serverPort_   = -1;
    char                buffer_[1024] = { 0 };
    XMsg                msg_;

    bool isRecvMsg = true; /// �Ƿ������Ϣ
};

XComTask::PImpl::PImpl(XComTask *owenr) : owenr_(owenr)
{
}

XComTask::PImpl::~PImpl() = default;

XComTask::XComTask()
{
    impl_ = std::make_shared<PImpl>(this);
}

XComTask::~XComTask() = default;

auto XComTask::init() -> bool
{
    /// ���ֿͻ��˺ͷ�����
    int comSock = this->sock();
    if (comSock <= 0)
        comSock = -1;

    /// ��bufferevent��������
    impl_->bev_ = bufferevent_socket_new(base(), comSock, BEV_OPT_CLOSE_ON_FREE);
    if (!impl_->bev_)
    {
        std::cerr << "bufferevent_socket_new failed" << std::endl;
        return false;
    }

    bufferevent_setcb(impl_->bev_, SReadCb, SWriteCb, SEventCb, this);
    bufferevent_enable(impl_->bev_, EV_READ | EV_WRITE);

    timeval tv = { 10, 0 };
    bufferevent_set_timeouts(impl_->bev_, &tv, &tv);

    /// ���ӷ�����
    if (impl_->serverIp_.empty())
    {
        return true;
    }

    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(impl_->serverPort_);
    evutil_inet_pton(AF_INET, impl_->serverIp_.c_str(), &sin.sin_addr.s_addr);

    int ret = bufferevent_socket_connect(impl_->bev_, (sockaddr *)&sin, sizeof(sin));
    if (ret < 0)
    {
        std::cerr << "bufferevent_socket_connect failed" << std::endl;
        return false;
    }

    return true;
}

void XComTask::setServerIp(const std::string ip)
{
    impl_->serverIp_ = ip;
}

void XComTask::setServerPort(int port)
{
    impl_->serverPort_ = port;
}

void XComTask::setServerRoot(const std::string path)
{
    impl_->serverPath_ = path;
}

void XComTask::setIsRecvMsg(bool isRecvMsg)
{
    impl_->isRecvMsg = isRecvMsg;
}

void XComTask::eventCB(short events)
{
    if (events & BEV_EVENT_CONNECTED)
    {
        std::cout << "BEV_EVENT_CONNECTED" << std::endl;

        /// ���ӳɹ�������Ϣ
        connectCB();
    }

    /// �˳�Ҫ����������
    if (events & (BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
    {
        std::cout << "BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT" << std::endl;
        close();
    }

    if (events & BEV_EVENT_EOF)
    {
        std::cout << "BEV_EVENT_EOF" << std::endl;
        close();
    }
}

void XComTask::connectCB()
{
    std::cout << "connectCB" << std::endl;
}

void XComTask::readCB()
{
    if (!impl_->bev_)
        return;


    /// ������Ϣ
    for (;;)
    {
        if (!impl_->isRecvMsg)
        {
            int len = bufferevent_read(impl_->bev_, impl_->buffer_, sizeof(impl_->buffer_));
            if (len <= 0)
                return;
            read(impl_->buffer_, len);
            continue;
        }

        /// ������Ϣͷ��
        if (!impl_->msg_.data)
        {
            int headSize = sizeof(XMsgHead);
            int len      = bufferevent_read(impl_->bev_, &impl_->msg_, headSize);
            if (len <= 0)
            {
                return;
            }
            if (len != headSize)
            {
                std::cerr << "read: msg head error!!!" << std::endl;
                return;
            }

            /// ��֤��Ϣͷ������Ч��
            if (impl_->msg_.type > MSG_MAX_TYPE || impl_->msg_.size <= 0 || impl_->msg_.size > MSG_MAX_SIZE)
            {
                std::cerr << "read: msg head error!!!" << std::endl;
                return;
            }
            impl_->msg_.data = new char[impl_->msg_.size];
        }

        /// ������Ϣ����
        int readSize = impl_->msg_.size - impl_->msg_.recved;
        if (readSize <= 0)
        {
            delete impl_->msg_.data;
            memset(&impl_->msg_, 0, sizeof(impl_->msg_));
            return;
        }
        int len = bufferevent_read(impl_->bev_, impl_->msg_.data + impl_->msg_.recved, readSize);
        if (len <= 0)
        {
            delete impl_->msg_.data;
            memset(&impl_->msg_, 0, sizeof(impl_->msg_));
            return;
        }
        impl_->msg_.recved += len;
        if (impl_->msg_.size == impl_->msg_.recved)
        {
            /// ������Ϣ
            if (!read(&impl_->msg_))
                return;

            delete impl_->msg_.data;
            memset(&impl_->msg_, 0, sizeof(impl_->msg_));
        }
    }
}

// void XComTask::read(const XMsg *msg)
// {
//     std::cout << "recv Msg type: " << msg->type << " size: " << msg->size << std::endl;
// }

void XComTask::read(void *data, int size)
{
}

void XComTask::writeCB()
{
    std::cout << "writeCB" << std::endl;
}

bool XComTask::write(const XMsg *msg)
{
    if (!impl_->bev_ || !msg || !msg->data || msg->size <= 0)
    {
        return false;
    }
    /// ������Ϣͷ
    int len = bufferevent_write(impl_->bev_, msg, sizeof(XMsgHead));
    if (len != 0)
    {
        return false;
    }

    /// ������Ϣ����
    len = bufferevent_write(impl_->bev_, msg->data, msg->size);
    if (len != 0)
    {
        return false;
    }
    return true;
}

bool XComTask::write(const void *data, int size)
{
    if (!impl_->bev_ || !data || size <= 0)
    {
        return false;
    }
    int len = bufferevent_write(impl_->bev_, data, size);
    if (len != 0)
    {
        return false;
    }
    return true;
}

void XComTask::beginWriteCB()
{
    if (!impl_->bev_)
        return;

    bufferevent_trigger(impl_->bev_, EV_WRITE, 0);
}

void XComTask::close()
{
    if (impl_->bev_)
    {
        bufferevent_free(impl_->bev_);
        impl_->bev_ = nullptr;
    }

    if (impl_->msg_.data)
    {
        delete impl_->msg_.data;
        memset(&impl_->msg_, 0, sizeof(impl_->msg_));
    }

    delete this;
}
