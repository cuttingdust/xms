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

public:
    void initGUI();

    /// \brief 加载配置项，从配置中心获取，并解析生成界面
    /// \param ip
    /// \param port
    /// \return
    bool loadConfig(const char *ip, int port);

    void loadProto(const char *filename, const char *class_name);

signals:
    void signalAddLog(const char *log);

    void signalMessageCB(bool is_ok, const char *msg);

protected slots:
    void slotSave();

    ///选择proto文件，并加载动态编译
    void slotLoadProto();

    void slotMessageCB(bool is_ok, const char *msg);

public:
    Ui::ConfigEdit *ui = nullptr;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // CONFIGEDIT_H
