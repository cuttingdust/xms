#include "TaskListGui.h"

#include "ui_task_list_gui.h"
#include "TaskItemGui.h"

#include <XDiskCom.pb.h>

class TaskListGui::PImpl
{
public:
    PImpl(TaskListGui *owenr);
    ~PImpl() = default;

public:
    TaskListGui                 *owenr_ = nullptr;
    Ui::TaskListGui             *ui_    = nullptr;
    std::map<int, TaskItemGui *> task_items_;

    std::list<xdisk::XFileTask> download_list;
    std::list<xdisk::XFileTask> upload_list;
};

TaskListGui::PImpl::PImpl(TaskListGui *owenr) : owenr_(owenr)
{
    ui_ = new Ui::TaskListGui;

    ui_->setupUi(owenr_);
    auto tab = ui_->taskableWidget;
    while (tab->rowCount() > 0)
    {
        tab->removeRow(0);
    }
    tab->setIconSize(QSize(30, 30));
    tab->setColumnWidth(0, 100); /// checkall
    tab->setColumnWidth(1, 500); /// filename

    tab->setSelectionBehavior(QAbstractItemView::SelectRows);  /// 设置选中模式为选中行
    tab->setSelectionMode(QAbstractItemView::SingleSelection); /// 设置选中单个
    ui_->uplabel->setText("");
    ui_->downlabel->setText("");
    ui_->oklabel->setText("");
}

TaskListGui::TaskListGui(QWidget *parent) : QWidget(parent)
{
    impl_ = std::make_unique<TaskListGui::PImpl>(this);
}

TaskListGui::~TaskListGui()
{
}

void TaskListGui::Show()
{
    this->show();
    QWidget *p       = dynamic_cast<QWidget *>(this->parent());
    int      w       = p->width();
    auto     tab_pos = impl_->ui_->taskableWidget->pos();
    auto     size    = impl_->ui_->taskableWidget->size();

    size.setHeight(p->height() - pos().y());
    size.setWidth(w - tab_pos.x());
    impl_->ui_->taskableWidget->resize(size);
}

void TaskListGui::Hide()
{
    this->hide();
}

void TaskListGui::Select(QModelIndex index)
{
}

void TaskListGui::RefreshUploadTask(std::list<xdisk::XFileTask> file_list)
{
    if (file_list.empty())
    {
        return;
    }

    std::stringstream ss;
    ss << "(" << file_list.size() << ")";
    impl_->ui_->uplabel->setText(ss.str().c_str());

    impl_->upload_list = file_list;
    if (!impl_->ui_->upButton->isChecked())
    {
        return;
    }

    RefreshTask(file_list);
}

void TaskListGui::RefreshDownloadTask(std::list<xdisk::XFileTask> file_list)
{
    if (file_list.empty())
    {
        return;
    }

    impl_->download_list = file_list;

    std::stringstream ss;
    ss << "(" << file_list.size() << ")";
    impl_->ui_->downlabel->setText(ss.str().c_str());

    if (!impl_->ui_->downButton->isChecked())
    {
        return;
    }

    RefreshTask(file_list);
}

void TaskListGui::RefreshTask(std::list<xdisk::XFileTask> file_list)
{
    /// 只修改 不清理
    auto tab = impl_->ui_->taskableWidget;
    //while (tab->rowCount() > 0)
    //{
    //    tab->removeRow(0);
    //}
    for (const auto &task : file_list)
    {
        if (!impl_->task_items_.contains(task.index()))
        {
            tab->insertRow(0);
            auto item = new TaskItemGui;
            item->setTask(task);
            tab->setCellWidget(0, 0, item);

            // item->show();
            tab->setRowHeight(0, 51);
            // tab->setRowHeight(0, item->height());
            impl_->task_items_[task.index()] = item;
        }
        else
        {
            impl_->task_items_[task.index()]->setTask(task);
        }
    }
}

void TaskListGui::OkTask()
{
    // RefreshTask(ok_list);
}

void TaskListGui::UpTask()
{
    auto tab = impl_->ui_->taskableWidget;
    impl_->task_items_.clear();
    while (tab->rowCount() > 0)
    {
        tab->removeRow(0);
    }
    RefreshTask(impl_->upload_list);
}

void TaskListGui::DownTask()
{
    impl_->task_items_.clear();
    auto tab = impl_->ui_->taskableWidget;
    while (tab->rowCount() > 0)
    {
        tab->removeRow(0);
    }
    RefreshTask(impl_->download_list);
}
