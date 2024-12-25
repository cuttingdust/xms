/**
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
#include "ui_config_gui.h"

class CongfigGui : public QWidget
{
    Q_OBJECT
public:
    explicit CongfigGui(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~CongfigGui() override;

public:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    Ui::ConfigGuiClass *ui;
};


#endif // CONGFIGGUI_H
