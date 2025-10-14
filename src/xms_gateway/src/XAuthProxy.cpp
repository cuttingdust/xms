#include "XAuthProxy.h"

#include <XTools.h>

#include <mutex>

static std::map<std::string, xmsg::XLoginRes> token_cache;
static std::mutex                             token_cache_mutex;

XAuthProxy::XAuthProxy()
{
}

XAuthProxy::~XAuthProxy()
{
}

auto XAuthProxy::initAuth() -> void
{
}

auto XAuthProxy::readCB(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    if (!head)
    {
        return;
    }

    xmsg::XLoginRes res;
    switch (head->msgtype())
    {
        case xmsg::MT_LOGIN_RES:
            {
                if (res.ParseFromArray(msg->data, msg->size))
                {
                    token_cache[res.token()] = res;
                }
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
        return false;
    }
    const auto tt = token_cache.find(token);
    if (tt == token_cache.end())
    {
        return false;
    }

    if (tt->second.username() != head->username())
    {
        return false;
    }

    if (tt->second.rolename() != head->username())
    {
        return false;
    }

    if (tt->second.expired_time() < time(0))
    {
        token_cache.erase(tt);
        return false;
    }

    return true;
}
