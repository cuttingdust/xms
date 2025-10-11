#include "XLoginGui.h"
#include "ui_xlogin_gui.h"

#include <XAuthClient.h>

#include <QtWidgets/QMessageBox>
#include <QtGui/QMouseEvent>

#include <string>

class XLoginGui::PImpl
{
public:
    PImpl(XLoginGui *owenr);
    ~PImpl() = default;

public:
    XLoginGui *owenr_  = nullptr;
    QPoint     curPos_ = { 0, 0 };
};

XLoginGui::PImpl::PImpl(XLoginGui *owenr) : owenr_(owenr)
{
}

XLoginGui::XLoginGui(QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags)
{
    impl_ = std::make_unique<XLoginGui::PImpl>(this);
    ui    = new Ui::XLoginGUI;
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->err_frame->hide();
}

XLoginGui::~XLoginGui() = default;

void XLoginGui::mouseMoveEvent(QMouseEvent *e)
{
    if ((e->buttons() & Qt::LeftButton) && e->pos().y() < ui->topwidget->height())
    {
        move(e->pos() + pos() - impl_->curPos_);
    }

    QWidget::mouseMoveEvent(e);
}

void XLoginGui::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        impl_->curPos_ = e->pos();
    }

    QWidget::mousePressEvent(e);
}

void XLoginGui::mouseReleaseEvent(QMouseEvent *event)
{
    impl_->curPos_ = { 0, 0 };
}


void XLoginGui::Login()
{
    ui->err_frame->show();
    const auto &username = ui->usernameEdit->text().toStdString();
    const auto &password = ui->passwordEdit->text().toStdString();
    if (username.empty() || password.empty())
    {
        ui->err_msg->setText("用户名或密码不能为空！");
        return;
    }
    XAuthClient::get()->loginReq(username, password);
    xmsg::XLoginRes res;
    if (XAuthClient::get()->getLoginInfo(username, &res, 2000))
    {
        ui->err_msg->setText("登录成功！");
        QDialog::accept();
        return;
    }
    ui->err_msg->setText("用户名密码验证有误！");
}
