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

#include <XDiskCom.pb.h>

#include <QtWidgets/QMainWindow>


class XFileManager;

namespace Ui
{
    class XDiskClientGui;
}

class XDiskClientGui : public QWidget
{
    Q_OBJECT
public:
    explicit XDiskClientGui(XFileManager *xfm, QWidget *parent = Q_NULLPTR);
    ~XDiskClientGui() override;

public slots:
    void Refresh();
    void RefreshData(xdisk::XFileInfoList file_list, std::string cur_dir);
    void Checkall();
    void NewDir();
    void Upload();

    void DoubleClicked(int row, int col);
    void Root();
    void Back();
    void Delete();
    void RefreshDiskInfo(xdisk::XDiskInfo info);

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    Ui::XDiskClientGui *ui = nullptr;

    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XDISKCLIENTGUI_H
