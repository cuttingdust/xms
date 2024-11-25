#include "XTestClient.h"
#include "XThreadPool.h"
#include <iostream>

#define SPORT 8080

int main(int argc, char *argv[])
{
    XTestClient *client = new XTestClient;
    client->setServerIp("127.0.0.1");
    client->setServerPort(SPORT);
    client->StartConnect();
    XThreadPool::wait();
    return 0;
}
