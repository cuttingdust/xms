/**
 * @file   XDiskClientGui.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-11
 */

#ifndef XDISKCLIENTGUI_H
#define XDISKCLIENTGUI_H

#include <QtWidgets/QWidget>

namespace Ui
{
    class XDiskClientGui;
}

class XDiskClientGui : public QWidget
{
    Q_OBJECT
public:
    explicit XDiskClientGui();
    ~XDiskClientGui() override;

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    Ui::XDiskClientGui *ui = nullptr;

    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XDISKCLIENTGUI_H
