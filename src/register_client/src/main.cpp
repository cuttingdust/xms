#include "XRegisterClient.h"

#include <XThreadPool.h>

#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "Register Client " << std::endl;
    std::cout << "register_client 127.0.0.1 20018 " << std::endl;
    /// ×¢²áÖÐÐÄIP ¶Ë¿Ú
    ///
    std::string ip   = "127.0.0.1";
    int         port = REGISTER_PORT;
    if (argc > 2)
    {
        ip   = argv[1];
        port = atoi(argv[2]);
    }

    XRegisterClient::get()->setServerIp(ip.c_str());
    XRegisterClient::get()->setServerPort(port);
    XRegisterClient::get()->registerServer("test", 20020, "127.0.0.1");
    XThreadPool::wait();
    return 0;
}
