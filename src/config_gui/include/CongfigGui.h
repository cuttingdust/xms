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

class CongfigGui : public QWidget
{
    Q_OBJECT
public:
    explicit CongfigGui(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~CongfigGui() override;

public:
    bool eventFilter(QObject *object, QEvent *event) override;

    void showEvent(QShowEvent *event) override;

    /// ��ʾ����־�б���
    void addLog(const char *log);

    /// \brief ˢ������
    void updateUI();
public Q_SLOTS:
    /// ˢ����ʾ����
    void slotRefresh();

    /// \brief ��������
    void slotAddConfig();

    /// \brief ɾ������
    void slotDeleteConfig();

    /// \brief �༭����
    void slotEditConfig();

private:
    Ui::ConfigGuiClass *ui = nullptr;
};


#endif // CONGFIGGUI_H
