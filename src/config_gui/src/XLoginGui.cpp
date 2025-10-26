#include "XLoginGui.h"
#include "ui_xlogin_gui.h"
#include "XConfigClient.h"

#include <XAuthClient.h>

#include <QtWidgets/QMessageBox>

#include <string>

XLoginGui::XLoginGui(QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags)
{
    ui = new Ui::XLoginGUI;
    ui->setupUi(this);
}

XLoginGui::~XLoginGui()
{
}

void XLoginGui::Login()
{
    if (ui->usernameEdit->text().isEmpty())
    {
        QMessageBox::information(this, "", "用户名不能为空");
        return;
    }
    if (ui->passwordEdit->text().isEmpty())
    {
        QMessageBox::information(this, "", "密码不能为空");
        return;
    }
    std::string username = ui->usernameEdit->text().toStdString();
    std::string password = ui->passwordEdit->text().toStdString();

    XAuthClient::get()->login(username, password);
    xmsg::XLoginRes login;
    bool            re = XAuthClient::get()->getLoginInfo(username, &login, 1000);
    if (!re)
    {
        QMessageBox::information(this, "", "用户名或密码错误");
        return;
    }
    std::cout << "Login success!" << std::endl;
    XConfigClient::get()->setLogin(&login);
    accept();
}
