#include "XLogDao.h"
#include "XLogServer.h"

#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "XLog Server " << std::endl;
    XLogDao::get()->init();
    XLogDao::get()->install();

    XLogServer server;
    server.setServerPort(XLOG_PORT);
    server.start();
    //XThreadPool::Wait();
    return 0;
}
