#include "ConfigEdit.h"
#include "ConfigGui.h"

#include <XConfigClient.h>
#include <XAuthClient.h>

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");
    QApplication app(argc, argv);

    // ConfigEdit edit;
    // edit.exec();
    // return 0;

    ConfigGui w;
    w.show();

    app.exec();
    return 0;
}
