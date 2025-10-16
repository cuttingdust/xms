#include "XDiskClientGui.h"

#include "ui_xdisk_client_gui.h"
#include "XDiskCom.pb.h"
#include "XFileManager.h"
#include "XTools.h"


#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QLineEdit>
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
    std::string     remote_dir_;
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

    impl_->xfm_->getDir(impl_->remote_dir_);
}

void XDiskClientGui::RefreshData(xdisk::XFileInfoList file_list, std::string cur_dir)
{
    QString view_dir = "";
    QString dir_str  = QString::fromStdString(cur_dir);
    auto    dir_list = dir_str.split("/");
    for (const auto &d : dir_list)
    {
        auto dir = d.trimmed();
        if (dir.isEmpty())
        {
            continue;
        }
        view_dir += dir;
        view_dir += "> ";
    }
    ui->dir_label->setText(view_dir);
    /// TODO 路径可以组装 然后点击某一个路径跳转

    auto tab = ui->filetableWidget;
    while (tab->rowCount() > 0)
    {
        tab->removeRow(0);
    }
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
    impl_->remote_dir_ = cur_dir;
}

void XDiskClientGui::Checkall()
{
    auto tab = ui->filetableWidget;
    for (int i = 0; i < tab->rowCount(); i++)
    {
        const auto w = tab->cellWidget(i, 0);
        if (!w)
        {
            continue;
        }

        const auto check = dynamic_cast<QCheckBox *>(w->layout()->itemAt(0)->widget());
        if (!check)
        {
            continue;
        }

        check->setChecked(ui->checkallBox->isChecked());
    }
}

void XDiskClientGui::NewDir()
{
    QDialog dialog;
    dialog.setWindowFlags(Qt::FramelessWindowHint);    /// 去除原窗口边框
    dialog.setAttribute(Qt::WA_TranslucentBackground); /// 隐藏背景，用于圆角
    dialog.resize(400, 50);
    QLineEdit edit(&dialog);
    edit.resize(300, 40);
    QPushButton ok(&dialog);
    ok.move(305, 0);
    ok.setText("确定");
    QPushButton cancel(&dialog);
    cancel.setText("取消");
    cancel.move(305, 22);
    connect(&cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
    connect(&ok, SIGNAL(clicked()), &dialog, SLOT(accept()));

    auto re = dialog.exec();
    if (re == QDialog::Rejected)
    {
        return;
    }

    std::string dir = edit.text().toStdString();
    if (dir.empty())
    {
        return;
    }

    impl_->xfm_->newDir(impl_->remote_dir_ + "/" + dir);
}

void XDiskClientGui::DoubleClicked(int row, int col)
{
    auto        item     = ui->filetableWidget->item(row, 1);
    QString     dir      = item->text();
    std::string filename = dir.toStdString();
    impl_->xfm_->getDir(impl_->remote_dir_ + "/" + filename);
}

void XDiskClientGui::Root()
{
    impl_->xfm_->getDir("");
}

void XDiskClientGui::Back()
{
    if (impl_->remote_dir_.empty() || impl_->remote_dir_ == "/")
    {
        return;
    }

    std::string tmp = impl_->remote_dir_;
    if (tmp[tmp.size() - 1] == '/')
    {
        tmp = tmp.substr(0, tmp.size() - 1);
    }
    int index          = tmp.find_last_of('/');
    impl_->remote_dir_ = tmp.substr(0, index);
    impl_->xfm_->getDir(impl_->remote_dir_);
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

void XDiskClientGui::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu context;
    context.addAction(ui->action_new_dir);
    context.addAction(ui->upaction);
    context.addAction(ui->downaction);
    context.addAction(ui->refreshaction);
    context.exec(QCursor::pos());
}
