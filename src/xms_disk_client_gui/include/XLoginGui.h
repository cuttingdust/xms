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

public:
    [[nodiscard]] auto getUserName() const -> std::string;

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

protected slots:
    void Login();

private:
    Ui::XLoginGUI *ui = nullptr;

    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XLOGINGUI_H
