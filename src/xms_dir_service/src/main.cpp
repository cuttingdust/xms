#include "XDirHandle.h"
#include "XDirService.h"
#include <XThreadPool.h>

#include <iostream>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");

    std::cout << "hello world" << std::endl;
    XDirHandle::regMsgCallback();
    XDirService xdir;
    xdir.setServerPort(DIR_PORT);
    xdir.start();
    XThreadPool::wait();
    return 0;
}
