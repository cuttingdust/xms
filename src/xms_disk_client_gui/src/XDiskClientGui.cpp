#include "XDiskClientGui.h"

#include "ui_xdisk_client_gui.h"

#include <QtWidgets/QMessageBox>
#include <QtGui/QMouseEvent>

class XDiskClientGui::PImpl
{
public:
    PImpl(XDiskClientGui *owenr);
    ~PImpl() = default;

public:
    XDiskClientGui *owenr_  = nullptr;
    QPoint          curPos_ = { 0, 0 };
};

XDiskClientGui::PImpl::PImpl(XDiskClientGui *owenr) : owenr_(owenr)
{
}

XDiskClientGui::XDiskClientGui()
{
    impl_ = std::make_unique<XDiskClientGui::PImpl>(this);
    ui    = new Ui::XDiskClientGui;
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
}

XDiskClientGui::~XDiskClientGui()
{
}

void XDiskClientGui::mouseMoveEvent(QMouseEvent *e)
{
    if ((e->buttons() & Qt::LeftButton) && e->pos().y() < ui->topwidget->height())
    {
        move(e->pos() + pos() - impl_->curPos_);
    }

    QWidget::mouseMoveEvent(e);
}

void XDiskClientGui::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        impl_->curPos_ = e->pos();
    }

    QWidget::mousePressEvent(e);
}

void XDiskClientGui::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    impl_->curPos_ = { 0, 0 };
}
