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

namespace Ui
{
    class ConfigGuiClass;
};

class ConfigGui : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigGui(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~ConfigGui() override;

public:
    bool eventFilter(QObject *object, QEvent *event) override;

    void showEvent(QShowEvent *event) override;

    /// 显示在日志列表中
    auto addLog(const char *log) -> void;

    auto checkLogin(std::string ip, int port) -> bool;

    /// \brief 刷新配置
    void updateUI();

signals:
    void SEditConfCB(void *config);

public Q_SLOTS:
    /// 刷新显示配置
    void slotRefresh();

    /// \brief 新增配置
    void slotAddConfig();

    /// \brief 删除配置
    void slotDeleteConfig();

    /// \brief 编辑配置
    void slotEditConfig();

    void EditConfCB(void *config);

private:
    Ui::ConfigGuiClass *ui_ = nullptr;
};


#endif // CONGFIGGUI_H
