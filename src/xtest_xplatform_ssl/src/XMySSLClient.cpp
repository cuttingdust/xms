#include "XMySSLClient.h"

XMySSLClient::XMySSLClient() = default;

XMySSLClient::~XMySSLClient() = default;

void XMySSLClient::connectCB()
{
    std::cout << "MySSLClient connected" << std::endl;
    xmsg::XLoginReq req;
    req.set_username("test_ssl_user");
    req.set_password("test_ssl_pass");
    sendMsg(xmsg::MT_LOGIN_REQ, &req);
}
