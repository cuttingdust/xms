﻿/**
 * @file   CongfigGui.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-25
 */

#ifndef CONGFIGGUI_H
#define CONGFIGGUI_H

#include <QtWidgets/QWidget>

namespace Ui
{
    class ConfigGuiClass;
};

class CongfigGui : public QWidget
{
    Q_OBJECT
public:
    explicit CongfigGui(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~CongfigGui() override;

public:
    bool eventFilter(QObject *object, QEvent *event) override;

    void showEvent(QShowEvent *event) override;

    /// 显示在日志列表中
    void addLog(const char *log);

    /// \brief 刷新配置
    void updateUI();
public Q_SLOTS:
    /// 刷新显示配置
    void slotRefresh();

    /// \brief 新增配置
    void slotAddConfig();

    /// \brief 删除配置
    void slotDeleteConfig();

    /// \brief 编辑配置
    void slotEditConfig();

private:
    Ui::ConfigGuiClass *ui = nullptr;
};


#endif // CONGFIGGUI_H
