#include "XMySSLClient.h"
#include "XMySSLServiceHandle.h"
#include "XMySSLService.h"

#include <thread>
#include <XSSL_CTX.h>
#include <XThreadPool.h>

#define IP   "127.0.0.1"
#define PORT 20300

int main(int argc, char *argv[])
{
    XSSL_CTX server_ctx;
    server_ctx.initServer("assert/server.crt", "assert/server.key");

    /// 测试SSL服务端
    XMySSLServiceHandle::regMsgCallback();
    XMySSLService service;
    service.setServerPort(PORT);
    service.setSSLContent(&server_ctx);
    service.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));


    /// 测试SSL客户端
    XMySSLClient client;
    client.setServerIp(IP);
    client.setServerPort(PORT);
    XSSL_CTX ctx;
    ctx.initClient();
    client.setSSLContent(&ctx);
    client.startConnect();

    XThreadPool::wait();

    return 0;
}
