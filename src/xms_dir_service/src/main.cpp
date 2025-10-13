#include "XDirHandle.h"
#include "XDirService.h"
#include <XThreadPool.h>

#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "hello world" << std::endl;
    XDirHandle::regMsgCallback();
    XDirService xdir;
    xdir.setServerPort(DIR_PORT);
    xdir.start();
    XThreadPool::wait();
    return 0;
}
