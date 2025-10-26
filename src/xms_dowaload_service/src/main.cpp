#include "XDownloadServer.h"

int main(int argc, char *argv[])
{
    XDownloadServer server;
    server.main(argc, argv);
    server.start();
    std::cout << DOWNLOAD_NAME << std::endl;
    server.wait();
    return 0;
}
