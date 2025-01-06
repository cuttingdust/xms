/**
 * @file   ConfigEdit.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-26
 */

#ifndef CONFIGEDIT_H
#define CONFIGEDIT_H

#include <QDialog>
namespace Ui
{
    class ConfigEdit;
}

class ConfigEdit : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigEdit(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~ConfigEdit() override;
signals:
    void signalAddLog(const char *log);

protected slots:
    void slotSave();

    ///选择proto文件，并加载动态编译
    void slotLoadProto();

public:
    Ui::ConfigEdit *ui = nullptr;
};


#endif // CONFIGEDIT_H
