#include "XMSFileManager.h"
#include "XLoginGui.h"
#include "XDiskClientGui.h"

#include <XTools.h>
#include <XAuthClient.h>

#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");
    XMSFileManager xfm;
    QApplication   a(argc, argv);

    const auto& gateway_ip   = XTools::XGetHostByName(API_GATEWAY_SERVER_NAME);
    const auto& gateway_port = XTools::XGetPortByName(API_GATEWAY_SERVER_NAME);
    XAuthClient::get()->setServerIP(gateway_ip.c_str());
    XAuthClient::get()->setServerPort(gateway_port); /// 客户端转发给api_gateway
    XAuthClient::get()->startConnect();

    XLoginGui login_gui;
    if (login_gui.exec() != QDialog::Accepted)
    {
        return -1;
    }
    xfm.initFileManager(gateway_ip, gateway_port);

    xfm.setLogin(XAuthClient::get()->getLogin());

    XDiskClientGui main_gui(&xfm);
    main_gui.show();

    /// 网关的地址 可以通过域名，在域名服务上再做负载均衡

    return a.exec();
}
