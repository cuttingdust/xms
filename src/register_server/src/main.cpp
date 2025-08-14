﻿#include "XRegisterServer.h"

#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "Register Server" << std::endl;
    XRegisterServer server;
    /// 初始化 传递参数，端口号 register_server 20018
    server.main(argc, argv);

    /// 启动服务线程，开始监听端口
    server.start();

    /// 阻塞，等待线程池退出
    server.wait();

    return 0;
}
