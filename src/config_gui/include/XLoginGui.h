/**
 * @file   XLoginGui.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-09-19
 */

#ifndef XLOGINGUI_H
#define XLOGINGUI_H

#include <QtWidgets/QDialog>

namespace Ui
{
    class XLoginGUI;
};

class XLoginGui : public QDialog
{
    Q_OBJECT
public:
    explicit XLoginGui(QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
    ~XLoginGui() override;

protected slots:
    void Login();

private:
    Ui::XLoginGUI *ui = nullptr;
};


#endif // XLOGINGUI_H
