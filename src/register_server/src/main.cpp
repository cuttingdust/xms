
#include "XRegisterServer.h"

#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "Register Server" << std::endl;
    XRegisterServer server;
    /// ��ʼ�� ���ݲ������˿ں� register_server 20018
    server.main(argc, argv);

    /// ���������̣߳���ʼ�����˿�
    server.start();

    /// �������ȴ��̳߳��˳�
    server.wait();

    return 0;
}
