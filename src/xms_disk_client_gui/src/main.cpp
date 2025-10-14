#include "XMSFileManager.h"
#include "XLoginGui.h"
#include "XDiskClientGui.h"

#include <XTools.h>
#include <XAuthClient.h>

#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");

    const auto& gateway_ip   = XTools::XGetHostByName(API_GATEWAY_SERVER_NAME);
    const auto& gateway_port = XTools::XGetPortByName(API_GATEWAY_SERVER_NAME);
    XAuthClient::get()->setServerIP(gateway_ip.c_str());
    XAuthClient::get()->setServerPort(gateway_port); /// 客户端转发给api_gateway
    XAuthClient::regMsgCallback();
    XAuthClient::get()->startConnect();

    QApplication a(argc, argv);
    XLoginGui    login_gui;
    if (login_gui.exec() != QDialog::Accepted)
    {
        return -1;
    }

    XMSFileManager xfm;
    xfm.initFileManager(gateway_ip, gateway_port);

    xmsg::XLoginRes login;
    if (XAuthClient::get()->getLoginInfo(login_gui.getUserName(), &login, 100))
    {
        LOGINFO(login.DebugString());
        xfm.setLogin(login);
    }

    XDiskClientGui main_gui(&xfm);
    main_gui.show();
    return a.exec();
}
