#include "XDiskClientGui.h"

#include "ui_xdisk_client_gui.h"
#include "XDiskCom.pb.h"
#include "XFileManager.h"

#include <QtWidgets/QMessageBox>
#include <QtGui/QMouseEvent>

class XDiskClientGui::PImpl
{
public:
    PImpl(XFileManager *xfm, XDiskClientGui *owenr);
    ~PImpl() = default;

public:
    XDiskClientGui *owenr_  = nullptr;
    QPoint          curPos_ = { 0, 0 };
    XFileManager   *xfm_    = nullptr;
};

XDiskClientGui::PImpl::PImpl(XFileManager *xfm, XDiskClientGui *owenr) : owenr_(owenr), xfm_(xfm)
{
}

XDiskClientGui::XDiskClientGui(XFileManager *xfm, QWidget *parent) : QWidget(parent)
{
    impl_ = std::make_unique<XDiskClientGui::PImpl>(xfm, this);
    ui    = new Ui::XDiskClientGui;
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<xdisk::XFileInfoList>("xdisk::XFileInfoList");
    connect(xfm, SIGNAL(RefreshData(xdisk::XFileInfoList, std::string)), this,
            SLOT(refreshData(xdisk::XFileInfoList, std::string)));

    auto tab = ui->filetableWidget;
    tab->setColumnWidth(0, 40);
    tab->setColumnWidth(1, 500);
    tab->setColumnWidth(2, 150);
    tab->setColumnWidth(3, 100);

    this->refresh();
}

XDiskClientGui::~XDiskClientGui()
{
}

void XDiskClientGui::refresh()
{
    if (!impl_->xfm_)
    {
        return;
    }

    impl_->xfm_->getDir("");
}

void XDiskClientGui::refreshData(xdisk::XFileInfoList file_list, std::string cur_dir)
{
    auto tab = ui->filetableWidget;
    while (tab->rowCount() > 0)
        tab->removeRow(0);
    for (const auto &file : file_list.files())
    {
        tab->insertRow(0);
        const std::string &filename = file.filename();
        const auto         qname    = filename.c_str();
        tab->setItem(0, 1, new QTableWidgetItem(qname));
        tab->setItem(0, 2, new QTableWidgetItem(file.filetime().c_str()));
    }
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
