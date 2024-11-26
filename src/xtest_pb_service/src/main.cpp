#include "XDirServiceHandle.h"

#include <XService.h>
#include <XThreadPool.h>
#include <XTools.h>

#include <iostream>

#define SPORT 8080

class XTestService : public XService
{
public:
    XServiceHandle *createHandle() override
    {
        return new XDirServiceHandle();
    }
};


int main(int argc, char *argv[])
{
    XTestService service;
    service.setServerPort(SPORT);
    service.start();
    XThreadPool::wait();

    return 0;
}
