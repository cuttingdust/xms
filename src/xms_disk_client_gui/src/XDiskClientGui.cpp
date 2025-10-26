#include "XDiskClientGui.h"

#include "ui_xdisk_client_gui.h"
#include "XFileManager.h"
#include "TaskListGui.h"
#include "FilePassword.h"

#include <XDiskCom.pb.h>
#include <XTools.h>

#include <QtWidgets/QFileDialog>
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

    std::list<QCheckBox *>      check_list_;
    std::list<xdisk::XFileInfo> file_list;
    TaskListGui                *task_gui_ = nullptr;
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
    auto head = ui->filetableWidget->horizontalHeader();
    head->setDefaultAlignment(Qt::AlignLeft);

    auto tab = ui->filetableWidget;
    tab->setColumnWidth(0, 40);  /// checkall
    tab->setColumnWidth(1, 300); /// filename
    tab->setColumnWidth(2, 150); /// time
    tab->setColumnWidth(3, 100); /// size
    auto hitem = tab->horizontalHeaderItem(0);

    qRegisterMetaType<xdisk::XFileInfoList>("xdisk::XFileInfoList");
    qRegisterMetaType<std::string>("std::string");
    connect(xfm, SIGNAL(RefreshData(xdisk::XFileInfoList, std::string)), this,
            SLOT(RefreshData(xdisk::XFileInfoList, std::string)));
    while (tab->rowCount() > 0)
    {
        tab->removeRow(0);
    }

    impl_->task_gui_ = new TaskListGui(this);
    impl_->task_gui_->hide();


    qRegisterMetaType<std::list<xdisk::XFileTask>>("std::list<xdisk::XFileTask>");
    connect(xfm, SIGNAL(RefreshUploadTask(std::list<xdisk::XFileTask>)), impl_->task_gui_,
            SLOT(RefreshUploadTask(std::list<xdisk::XFileTask>)));
    connect(xfm, SIGNAL(RefreshDownloadTask(std::list<xdisk::XFileTask>)), impl_->task_gui_,
            SLOT(RefreshDownloadTask(std::list<xdisk::XFileTask>)));


    qRegisterMetaType<xdisk::XDiskInfo>("xdisk::XDiskInfo");
    connect(xfm, SIGNAL(RefreshDiskInfo(xdisk::XDiskInfo)), this, SLOT(RefreshDiskInfo(xdisk::XDiskInfo)));
    connect(xfm, SIGNAL(ErrorSig(std::string)), this, SLOT(ErrorSlot(std::string)));

    this->Refresh();

    /// 显示用户名
    ui->username_label->setText(xfm->getLogin().username().c_str());
}

XDiskClientGui::~XDiskClientGui() = default;

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
    ui->username_label->setText(impl_->xfm_->getLogin().username().c_str());

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
        const std::string &filename = file.filename();
        if (filename.empty())
        {
            continue;
        }

        /// 文件类型
        //string filetype = "";
        //int pos = filename.find_last_of('.');
        //if (pos > 0)
        //{
        //    filetype = filename.substr(pos + 1);
        //}
        ////转换为小写 ，第三个参数是输出
        //transform(filetype.begin(), filetype.end(), filetype.begin(), ::tolower);

        /// 文件类型对应图标
        ///  :/XMSDiskClientGui/Resources/img/FileType/Small/DocType.png
        std::string iconpath = FILE_ICON_PATH;
        // map<string, string> icons;
        // icons["jpg"] = "Img";
        // icons["png"] = "Img";
        // icons["gif"] = "Img";

        iconpath += XTools::XGetIconFilename(filename, file.is_dir());
        iconpath += "Type.png";
        tab->insertRow(0);

        /// 第一列选择框 居中对齐
        auto ckb = new QCheckBox(tab);
        impl_->check_list_.push_back(ckb);
        auto hLayout = new QHBoxLayout;
        auto widget  = new QWidget(tab);
        hLayout->addWidget(ckb);
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->setSpacing(0);
        hLayout->setAlignment(ckb, Qt::AlignCenter);
        widget->setLayout(hLayout);
        tab->setCellWidget(0, 0, widget);

        /// 设定文件名和图标
        const auto qname = filename.c_str();
        tab->setItem(0, 1, new QTableWidgetItem(QIcon(iconpath.c_str()), qname));

        /// 文件时间
        tab->setItem(0, 2, new QTableWidgetItem(file.filetime().c_str()));

        /// 文件大小 B KB MB GB
        if (!file.is_dir())
        {
            /// 大小
            tab->setItem(0, 3, new QTableWidgetItem(XTools::XGetSizeString(file.filesize()).c_str()));
        }
    }
    /// 文件数量
    std::stringstream ss;
    ss << tab->rowCount();
    ui->file_count->setText(ss.str().c_str());

    impl_->remote_dir_ = cur_dir;
}

void XDiskClientGui::Checkall()
{
    static int count = 0;
    count++;
    qDebug() << count << "Checkall()" << ui->checkallBox->isChecked();
    auto tab = ui->filetableWidget;
    //for (auto check : check_list)
    //{
    //    check->setChecked(true);
    //}
    for (int i = 0; i < tab->rowCount(); i++)
    {
        const auto w = tab->cellWidget(i, 0);
        if (!w)
        {
            continue;
        }

        auto check = static_cast<QCheckBox *>(w->layout()->itemAt(0)->widget());
        //auto check = (QCheckBox*)tab->cellWidget(i, 0);
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

void XDiskClientGui::Upload()
{
    /// 用户选择一个文件
    QString filepath = QFileDialog::getOpenFileName(this, "请选择上传文件");
    if (filepath.isEmpty())
    {
        return;
    }

    qDebug() << "filepath:" << filepath << Qt::endl;

    QFileInfo fileinfo;
    fileinfo = QFileInfo(filepath);
    qDebug() << fileinfo.filePath() << Qt::endl;
    qDebug() << fileinfo.fileName() << Qt::endl;
    qDebug() << fileinfo.canonicalFilePath() << Qt::endl;
    qDebug() << fileinfo.absoluteFilePath() << Qt::endl;
    std::string      file_real_path = filepath.toStdString();
    std::string      filename       = fileinfo.fileName().toStdString();
    std::string      filedir        = file_real_path.substr(0, file_real_path.size() - filename.size());
    xdisk::XFileInfo task;
    task.set_filename(filename);
    task.set_filedir(impl_->remote_dir_);
    task.set_local_path(file_real_path);
    impl_->xfm_->uploadFile(task);
}

void XDiskClientGui::Download()
{
    // int row = ui.filetableWidget->currentRow();

    const auto tab = ui->filetableWidget;
    int        row = -1;
    for (int i = 0; i < tab->rowCount(); i++)
    {
        const auto w = tab->cellWidget(i, 0);
        if (!w)
        {
            continue;
        }

        auto check = static_cast<QCheckBox *>(w->layout()->itemAt(0)->widget());
        //auto check = (QCheckBox*)tab->cellWidget(i, 0);
        if (!check)
        {
            continue;
        }

        if (check->isChecked())
        {
            row = i;
            break;
        }
    }

    if (row < 0)
    {
        QMessageBox::information(this, "", "请选择下载文件");
        return;
    }
    /// 获取选择的文件名
    auto        item     = ui->filetableWidget->item(row, 1);
    std::string filename = item->text().toStdString();
    /// 获取下载路径
    QString localpath = QFileDialog::getExistingDirectory(this, "请选择下载路径");
    if (localpath.isEmpty())
    {
        return;
    }

    xdisk::XFileInfo task;
    task.set_filename(filename);
    task.set_filedir(impl_->remote_dir_);
    if (localpath[localpath.size() - 1] != "/" && localpath[localpath.size() - 1] != "\\")
    {
        localpath += "/";
    }

    task.set_local_path(localpath.toStdString() + filename);
    impl_->xfm_->downloadFile(task);
}

void XDiskClientGui::DoubleClicked(int row, int col)
{
    /// 双击，后面要考虑预览图片和视频
    auto        item     = ui->filetableWidget->item(row, 1);
    QString     dir      = item->text();
    std::string filename = dir.toStdString();
    impl_->xfm_->getDir(impl_->remote_dir_ + "/" + filename);
    qDebug() << item;
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

void XDiskClientGui::Delete()
{
    auto tab = ui->filetableWidget;
    int  row = -1;
    for (int i = 0; i < tab->rowCount(); i++)
    {
        const auto w = tab->cellWidget(i, 0);
        if (!w)
        {
            continue;
        }

        auto check = static_cast<QCheckBox *>(w->layout()->itemAt(0)->widget());
        //auto check = (QCheckBox*)tab->cellWidget(i, 0);
        if (!check)
        {
            continue;
        }

        if (check->isChecked())
        {
            row = i;
            break;
        }
    }
    if (row < 0)
    {
        QMessageBox::information(this, "", "请选择删除文件");
        return;
    }
    const auto re = QMessageBox::information(this, "", "您确认删除文件吗？", QMessageBox::Ok | QMessageBox::Cancel);
    if (re & QMessageBox::Cancel)
        return;

    auto             item     = ui->filetableWidget->item(row, 1);
    std::string      filename = item->text().toStdString();
    xdisk::XFileInfo file;
    file.set_filename(filename);
    //file.set_filedir()
    impl_->xfm_->deleteFile(file);
}

void XDiskClientGui::RefreshDiskInfo(xdisk::XDiskInfo info)
{
    ///  121MB/10GB
    std::string size_str = XTools::XGetSizeString(info.dir_size());
    size_str += "/";
    size_str += XTools::XGetSizeString(info.total());
    ui->disk_info_text->setText(size_str.c_str());
    ui->disk_info_bar->setMaximum(info.total());
    ui->disk_info_bar->setValue(info.dir_size());
}

void XDiskClientGui::SelectFile(QModelIndex index)
{
    const auto tab = ui->filetableWidget;
    for (int i = 0; i < tab->rowCount(); i++)
    {
        const auto w = tab->cellWidget(i, 0);
        if (!w)
        {
            continue;
        }

        auto check = static_cast<QCheckBox *>(w->layout()->itemAt(0)->widget());
        if (!check)
        {
            continue;
        }

        check->setChecked(false);
    }
    const auto w = tab->cellWidget(index.row(), 0);
    if (!w)
    {
        return;
    }

    auto check = static_cast<QCheckBox *>(w->layout()->itemAt(0)->widget());
    //auto check = (QCheckBox*)tab->cellWidget(i, 0);
    if (!check)
    {
        return;
    }

    check->setChecked(true);
}

void XDiskClientGui::TaskTab()
{
    impl_->task_gui_->move(ui->filelistwidget->pos().x(), ui->filelistwidget->pos().y());
    impl_->task_gui_->resize(size());
    ui->filelistwidget->hide();

    impl_->task_gui_->Show();
}

void XDiskClientGui::MyTab()
{
    if (!impl_->task_gui_)
    {
        return;
    }

    ui->filelistwidget->show();
    impl_->task_gui_->hide();
}

void XDiskClientGui::ErrorSlot(std::string err)
{
    QMessageBox::information(this, "XMS ERROR", err.c_str());
}

/// 文件是否加密上传
void XDiskClientGui::FileEnc()
{
    if (ui->file_enc->isChecked())
    {
        FilePassword pass_dia;
        if (pass_dia.exec() == QDialog::Accepted)
        {
            impl_->xfm_->set_password(pass_dia.getPassWd());
        }
    }
    else
    {
        impl_->xfm_->set_password("");
    }
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
