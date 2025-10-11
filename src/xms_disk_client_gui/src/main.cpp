#include "XLoginGui.h"
#include "XTools.h"

#include <XAuthClient.h>

#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");

    auto ip = XTools::XGetHostByName(API_GATEWAY_SERVER_NAME);
    XAuthClient::get()->setServerIp(ip.c_str());
    XAuthClient::get()->setServerPort(AUTH_PORT);
    XAuthClient::regMsgCallback();
    XAuthClient::get()->startConnect();

    QApplication a(argc, argv);
    XLoginGui    gui;
    if (gui.exec() != QDialog::Accepted)
    {
        return -1;
    }
    // XMSDiskClientGui main_gui;
    // main_gui.show();
    return a.exec();
    return 0;
}
