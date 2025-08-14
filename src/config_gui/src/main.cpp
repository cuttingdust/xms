#include "ConfigEdit.h"
#include "CongfigGui.h"

#include <XConfigClient.h>

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    XConfigClient::get()->startGetConf("127.0.0.1", CONFIG_PORT, 0, 0, 0);
    // ConfigEdit edit;
    // edit.exec();
    // return 0;

    CongfigGui w;
    w.show();

    app.exec();
    return 0;
}
