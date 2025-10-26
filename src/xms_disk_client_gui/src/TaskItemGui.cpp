#include "TaskItemGui.h"

#include "ui_task_item_gui.h"

#include <XTools.h>
#include <XDiskCom.pb.h>

#define FILE_MID_ICON_PATH ":/XMSDiskClientGui/Resources/img/FileType/Middle/"

class TaskItemGui::PImpl
{
public:
    PImpl(TaskItemGui *owenr);
    ~PImpl() = default;

public:
    TaskItemGui     *owenr_ = nullptr;
    Ui::TaskItemGui *ui_    = nullptr;
};

TaskItemGui::PImpl::PImpl(TaskItemGui *owenr) : owenr_(owenr)
{
    ui_ = new Ui::TaskItemGui;
    ui_->setupUi(owenr_);
}

TaskItemGui::TaskItemGui(QWidget *parent) : QWidget(parent)
{
    impl_ = std::make_unique<TaskItemGui::PImpl>(this);
}

TaskItemGui::~TaskItemGui() = default;

auto TaskItemGui::setTask(xdisk::XFileTask task) -> void
{
    auto file = task.file();

    /// 任务事件
    impl_->ui_->filetime->setText(task.tasktime().c_str());

    /// 文件名
    QString filename = QString::fromStdString(task.file().filename());
    impl_->ui_->filename->setText(filename);

    /// 文件大小
    std::stringstream ss;
    if (file.filesize() == 0)
    {
        ss << "0B" << std::endl;
    }
    else
    {
        ss << XTools::XGetSizeString(file.net_size()) << "/" << XTools::XGetSizeString(file.filesize());
    }


    impl_->ui_->filesize->setText(ss.str().c_str());

    /// 传输进度
    impl_->ui_->progressBar->setMinimum(0);
    long long filesize = file.filesize();
    long long netsize  = file.net_size();
    if (filesize > 1024 * 1024 * 10)
    {
        filesize /= 1000;
        netsize /= 1000;
    }
    impl_->ui_->progressBar->setMaximum(filesize);
    impl_->ui_->progressBar->setValue(netsize);

    ///文件图标
    std::string iconpath = FILE_MID_ICON_PATH;
    iconpath += XTools::XGetIconFilename(task.file().filename(), file.is_dir());
    iconpath += "Type.png";
    QString sty = "background-color: rgba(0, 0, 0,0);\n";
    sty += "background-image: url(";
    sty += iconpath.c_str();
    sty += ");";
    impl_->ui_->filetype->setStyleSheet(sty);
}
