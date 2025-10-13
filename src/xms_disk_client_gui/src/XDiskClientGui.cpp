#include "XDiskClientGui.h"

#include "ui_xdisk_client_gui.h"
#include "XDiskCom.pb.h"
#include "XFileManager.h"
#include "XTools.h"


#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtGui/QMouseEvent>

#define FILE_ICON_PATH ":/XMSDiskClientGui/Resources/img/FileType/Small/"

class XDiskClientGui::PImpl
{
public:
    PImpl(XFileManager *xfm, XDiskClientGui *owenr);
    ~PImpl() = default;

public:
    XDiskClientGui *owenr_  = nullptr;
    QPoint          curPos_ = { 0, 0 }; ///< 鼠标的位置
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
            SLOT(RefreshData(xdisk::XFileInfoList, std::string)));

    auto tab = ui->filetableWidget;
    tab->setColumnWidth(0, 40);
    tab->setColumnWidth(1, 300);
    tab->setColumnWidth(2, 150);
    tab->setColumnWidth(3, 100);

    this->Refresh();
}

XDiskClientGui::~XDiskClientGui()
{
}

void XDiskClientGui::Refresh()
{
    if (!impl_->xfm_)
    {
        return;
    }

    impl_->xfm_->getDir("");
}

void XDiskClientGui::RefreshData(xdisk::XFileInfoList file_list, std::string cur_dir)
{
    auto tab = ui->filetableWidget;
    while (tab->rowCount() > 0)
        tab->removeRow(0);
    for (const auto &file : file_list.files())
    {
        tab->insertRow(0);

        auto ckb     = new QCheckBox(tab);
        auto hLayout = new QHBoxLayout;
        auto widget  = new QWidget(tab);
        hLayout->addWidget(ckb);
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->setSpacing(0);
        hLayout->setAlignment(ckb, Qt::AlignCenter);
        widget->setLayout(hLayout);
        tab->setCellWidget(0, 0, widget);

        const std::string &filename = file.filename();

        ///  :/XMSDiskClientGui/Resources/img/FileType/Small/DocType.png
        std::string iconpath = FILE_ICON_PATH;
        iconpath += XTools::XGetIconFilename(filename, file.is_dir());
        iconpath += "Type.png";

        const auto qname = filename.c_str();
        tab->setItem(0, 1, new QTableWidgetItem(QIcon(iconpath.c_str()), qname));
        tab->setItem(0, 2, new QTableWidgetItem(file.filetime().c_str()));

        if (!file.is_dir())
        {
            /// 大小
            tab->setItem(0, 3, new QTableWidgetItem(XTools::XGetSizeString(file.filesize()).c_str()));
        }
    }
}

void XDiskClientGui::Checkall()
{
    auto tab = ui->filetableWidget;
    for (int i = 0; i < tab->rowCount(); i++)
    {
        auto w = tab->cellWidget(i, 0);
        if (!w)
            continue;
        auto check = dynamic_cast<QCheckBox *>(w->layout()->itemAt(0)->widget());
        if (!check)
            continue;
        check->setChecked(ui->checkallBox->isChecked());
    }
}

bool XDiskClientGui::eventFilter(QObject *watched, QEvent *event)
{
    return false;
}

void XDiskClientGui::mouseMoveEvent(QMouseEvent *e)
{
    if ((e->buttons() & Qt::LeftButton) && e->pos().y() < ui->topwidget->height())
    {
        move(e->pos() + pos() - impl_->curPos_);
    }
}

void XDiskClientGui::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        impl_->curPos_ = e->pos();
    }
}

void XDiskClientGui::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    impl_->curPos_ = { 0, 0 };
}

// void XDiskClientGui::resizeEvent(QResizeEvent *event)
// {
//     ui->filelistwidget->resize(event->size());
// }
