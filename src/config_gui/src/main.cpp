#include "CongfigGui.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    CongfigGui w;
    w.show();

    app.exec();
    return 0;
}
