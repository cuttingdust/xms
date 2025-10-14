#include "XDirHandle.h"
#include "XDirService.h"


#include <XTools.h>
#include <XThreadPool.h>
#include <XRegisterClient.h>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");

    std::string register_ip = XTools::XGetHostByName(REGISTER_SERVER_NAME);
    XRegisterClient::get()->setServerIP(register_ip.c_str());
    XRegisterClient::get()->registerServer(DIR_NAME, DIR_PORT, 0);

    XDirHandle::regMsgCallback();
    XDirService xdir;
    xdir.setServerPort(DIR_PORT);
    xdir.start();
    XThreadPool::wait();
    return 0;
}
