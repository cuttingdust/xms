#include "ConfigEdit.h"
#include "CongfigGui.h"

#include <XConfigClient.h>
#include <XAuthClient.h>

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");

    XConfigClient::get()->startGetConf("127.0.0.1", CONFIG_PORT, 0, 0, 0);
    XAuthClient::get()->setServerIp("127.0.0.1");
    XAuthClient::get()->setServerPort(AUTH_PORT);
    XAuthClient::regMsgCallback();
    XAuthClient::get()->startConnect();

    QApplication app(argc, argv);

    // ConfigEdit edit;
    // edit.exec();
    // return 0;

    CongfigGui w;
    w.show();

    app.exec();
    return 0;
}
