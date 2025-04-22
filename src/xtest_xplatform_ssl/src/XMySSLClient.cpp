#include "XMySSLClient.h"

XMySSLClient::XMySSLClient() = default;

XMySSLClient::~XMySSLClient() = default;

void XMySSLClient::connectCB()
{
    std::cout << "MySSLClient connected" << std::endl;
    this->write("OK", 3);
}
