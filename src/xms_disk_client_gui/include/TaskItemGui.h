/**
 * @file   TaskItemGui.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-24
 */

#ifndef TASKITEMGUI_H
#define TASKITEMGUI_H

#include <QtWidgets/QtWidgets>

namespace xdisk
{
    class XFileTask;
}

class TaskItemGui : public QWidget
{
public:
    explicit TaskItemGui(QWidget *parent = Q_NULLPTR);
    ~TaskItemGui() override;

public:
    auto setTask(xdisk::XFileTask task) -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // TASKITEMGUI_H
