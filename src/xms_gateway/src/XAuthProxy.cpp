#include "XAuthProxy.h"

#include <XTools.h>

#include <mutex>

static std::map<std::string, xmsg::XLoginRes> token_cache;
static std::mutex                             token_cache_mutex;

XAuthProxy::XAuthProxy() = default;

XAuthProxy::~XAuthProxy()
{
}

class TokenThread
{
public:
    void start()
    {
        std::thread th(&TokenThread::Main, this);
        th.detach();
    }
    ~TokenThread()
    {
        is_exit_ = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

private:
    bool is_exit_ = false;
    void Main()
    {
        while (!is_exit_)
        {
            token_cache_mutex.lock();
            /// 检测过期token 需要优化
            auto ptr = token_cache.begin();
            for (; ptr != token_cache.end();)
            {
                auto tmp = ptr;
                auto tt  = time(0);
                ptr++;
                if (tmp->second.expired_time() < tt)
                {
                    std::cout << "expired_time " << tmp->second.expired_time() << std::endl;
                    token_cache.erase(tmp);
                }
            }
            token_cache_mutex.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};

static TokenThread token_thread;
auto               XAuthProxy::initAuth() -> void
{
    token_thread.start();
}

auto XAuthProxy::readCB(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    if (!head)
    {
        return;
    }
    /// 如果是登录验证，和token延期 则本地保存
    /// 前期先只验证token有效，后面再验证权限
    xmsg::XLoginRes res;
    switch (head->msgtype())
    {
        case xmsg::MT_LOGIN_RES: /// 登录响应
            {
                if (res.ParseFromArray(msg->data, msg->size))
                {
                    XMutex mux(&token_cache_mutex);
                    token_cache[res.token()] = res;
                }
                std::cout << res.DebugString();
            }
        default:
            break;
    }

    XServiceProxyClient::readCB(head, msg);
}

auto XAuthProxy::checkToken(const xmsg::XMsgHead *head) -> bool
{
    XMutex      mux(&token_cache_mutex);
    std::string token = head->token();
    if (token.empty())
    {
        LOGINFO("XAuthProxy::CheckToken failed! token is empty!");
        return false;
    }
    const auto tt = token_cache.find(token);
    if (tt == token_cache.end())
    {
        LOGINFO("XAuthProxy::CheckToken failed! token is empty!");
        return false;
    }

    if (tt->second.username() != head->username())
    {
        std::stringstream ss;
        ss << "XAuthProxy::CheckToken failed! username is error!(head/cache)" << head->username();
        ss << "/" << tt->second.username();
        LOGINFO(ss.str().c_str());
        return false;
    }

    if (tt->second.rolename() != head->username())
    {
        std::stringstream ss;
        ss << "XAuthProxy::CheckToken failed! rolename is error!(head/cache)" << head->rolename();
        ss << "/" << tt->second.rolename();
        LOGINFO(ss.str().c_str());
        return false;
    }

    // if (tt->second.expired_time() < time(0))
    // {
    //     token_cache.erase(tt);
    //     return false;
    // }

    return true;
}
