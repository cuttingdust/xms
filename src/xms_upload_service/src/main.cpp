#include "XUploadService.h"
#include "XUploadHandle.h"

int main(int argc, char *argv[])
{
    XUploadService server;
    server.main(argc, argv);
    server.start();
    std::cout << UPLOAD_NAME << std::endl;
    XUploadService::wait();

    return 0;
}
