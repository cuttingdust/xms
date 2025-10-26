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
    XLoginGui  *owenr_  = nullptr;
    QPoint      curPos_ = { 0, 0 };
    std::string username_;
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

auto XLoginGui::getUserName() const -> std::string
{
    return impl_->username_;
}

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

void XLoginGui::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
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

    if (!XAuthClient::get()->login(username, password))
    {
        ui->err_msg->setText("用户名或者密码有误！");
        return;
    }
    static int count = 0;
    count++;

    ui->err_msg->setText(QString::number(count) + "登录成功！");
    QDialog::accept();
}
