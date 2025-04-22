#include "XMySSLClient.h"

#include <XSSL_CTX.h>
#include <XThreadPool.h>

#define IP   "127.0.0.1"
#define PORT 20030

int main(int argc, char *argv[])
{
    XMySSLClient client;
    client.setServerIp(IP);
    client.setServerPort(PORT);
    XSSL_CTX ctx;
    ctx.initClient();
    client.set_ssl_ctx(&ctx);
    client.startConnect();

    XThreadPool::wait();

    return 0;
}
