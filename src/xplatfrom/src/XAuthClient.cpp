#include "XAuthClient.h"

#include "XTools.h"

class XAuthClient::PImpl
{
public:
    PImpl(XAuthClient *owenr);
    ~PImpl() = default;

public:
    XAuthClient *owenr_ = nullptr;

    std::map<std::string, xmsg::XLoginRes> login_map_;
    std::mutex                             logins_mutex_;
};

XAuthClient::PImpl::PImpl(XAuthClient *owenr) : owenr_(owenr)
{
}

auto XAuthClient::get() -> XAuthClient *
{
    static XAuthClient xc;
    return &xc;
}

XAuthClient::XAuthClient()
{
    impl_ = std::make_unique<PImpl>(this);
}

XAuthClient::~XAuthClient() = default;

auto XAuthClient::loginReq(std::string username, std::string password) -> void
{
    xmsg::XLoginReq req;
    req.set_username(username);
    auto md5_pass = XTools::XMD5_base64(reinterpret_cast<unsigned char *>(password.data()), password.size());
    req.set_password(md5_pass);
    std::cout << req.DebugString();
    {
        XMutex mux(&impl_->logins_mutex_);
        impl_->login_map_.erase(username);
    }
    sendMsg(xmsg::MT_LOGIN_REQ, &req);
}

auto XAuthClient::addUserReq(xmsg::XAddUserReq *user) -> void
{
    auto md5_pass = XTools::XMD5_base64((unsigned char *)user->password().data(), user->password().size());
    user->set_password(md5_pass);
    sendMsg(xmsg::MT_ADD_USER_REQ, user);
}

bool XAuthClient::getLoginInfo(std::string username, xmsg::XLoginRes *out_info, int timeout_ms)
{
    if (!out_info)
        return false;
    int count = timeout_ms / 10;
    if (count <= 0)
        count = 1;
    int i = 0;
    for (; i < count; i++)
    {
        impl_->logins_mutex_.lock();
        auto login_ptr = impl_->login_map_.find(username);
        if (login_ptr == impl_->login_map_.end())
        {
            impl_->logins_mutex_.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        auto login = login_ptr->second;
        impl_->logins_mutex_.unlock();
        if (login.restype() != xmsg::XLoginRes::XRT_OK)
        {
            return false;
        }
        out_info->CopyFrom(login);
        return true;
    }
    return false;
}

auto XAuthClient::regMsgCallback() -> void
{
    regCB(xmsg::MT_LOGIN_RES, static_cast<MsgCBFunc>(&XAuthClient::loginRes));
    regCB(xmsg::MT_ADD_USER_RES, static_cast<MsgCBFunc>(&XAuthClient::addUserRes));
}

void XAuthClient::addUserRes(xmsg::XMsgHead *head, XMsg *msg)
{
    xmsg::XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        return;
    }
    if (res.return_() == xmsg::XMessageRes::XR_ERROR)
    {
        std::cout << res.msg() << std::endl;
        return;
    }
    std::cout << "Adduser success!" << std::endl;
}

void XAuthClient::loginRes(xmsg::XMsgHead *head, XMsg *msg)
{
    xmsg::XLoginRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        return;
    }
    std::cout << res.DebugString();
    {
        XMutex mux(&impl_->logins_mutex_);
        if (res.username().empty())
            return;
        impl_->login_map_[res.username()] = res;
    }
}
