#include "XDirServiceHandle.h"

#include <XService.h>
#include <XThreadPool.h>
#include <XTools.h>

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
    int server_port = 20011;
    if (argc > 1)
        server_port = atoi(argv[1]);
    std::cout << "server port is " << server_port << std::endl;

    XDirServiceHandle::regMsgCallback();

    XTestService service;
    service.setServerPort(server_port);
    service.start();
    XThreadPool::wait();

    return 0;
}
