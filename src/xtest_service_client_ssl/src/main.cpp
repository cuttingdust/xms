#include "XTestClient.h"

#include <XSSL_CTX.h>

#include <iostream>
#include <thread>

int main(int argc, char *argv[])
{
    XTestClient::regMsgCallback();

    XTestClient *client = new XTestClient;
    XSSL_CTX     ctx;
    ctx.initClient();
    client->setSSLContent(&ctx);
    client->setServerIP("127.0.0.1");
    client->setServerPort(API_GATEWAY_PORT);
    client->startConnect();

    for (int i = 0; i < 100000; ++i)
    {
        std::stringstream ss;
        ss << "/root/" << i;
        client->getDir(ss.str());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    client->wait();
    return 0;
}
