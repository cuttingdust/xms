#include "XDirHandle.h"
#include "XDirService.h"

#include <XTools.h>
#include <XRegisterClient.h>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");
    std::cout << "xms_dir_service!\n";
    std::string register_ip = XTools::XGetHostByName(API_REGISTER_SERVER_NAME);
    XRegisterClient::get()->setServerIP(register_ip.c_str());
    XRegisterClient::get()->registerServer(DIR_NAME, DIR_PORT, 0);

    XDirHandle::regMsgCallback();
    XDirService xdir;
    xdir.setServerPort(DIR_PORT);
    xdir.start();
    xdir.wait();
    return 0;
}
