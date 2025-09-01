#include "XAuthClient.h"

#include <XTools.h>

XAuthClient::XAuthClient() = default;

XAuthClient::~XAuthClient() = default;

void XAuthClient::LoginReq(std::string username, std::string password)
{
    xmsg::XLoginReq req;
    req.set_username(username);
    auto md5_pass = XTools::XMD5_base64(reinterpret_cast<unsigned char *>(password.data()), password.size());
    req.set_password(md5_pass);
    std::cout << req.DebugString();
    sendMsg(xmsg::MT_LOGIN_REQ, &req);
}
