/**
 * @file   TaskListGui.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-24
 */

#ifndef TASKLISTGUI_H
#define TASKLISTGUI_H

#include <QtWidgets/QtWidgets>

namespace xdisk
{
    class XFileTask;
}

class TaskListGui : public QWidget
{
    Q_OBJECT
public:
    explicit TaskListGui(QWidget *parent = Q_NULLPTR);
    ~TaskListGui() override;
    void Show();
    void Hide();
public slots:
    void Select(QModelIndex index);
    void RefreshUploadTask(std::list<xdisk::XFileTask> file_list);
    void RefreshDownloadTask(std::list<xdisk::XFileTask> file_list);
    void RefreshTask(std::list<xdisk::XFileTask> file_list);
    void OkTask();
    void UpTask();
    void DownTask();

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // TASKLISTGUI_H
