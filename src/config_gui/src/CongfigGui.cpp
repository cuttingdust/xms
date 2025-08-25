#include "CongfigGui.h"

#include "ConfigEdit.h"
#include "ui_config_gui.h"


#include <XTools.h>
#include <XConfigClient.h>

#include <QtGui/QtEvents>
#include <QtCore/QTime>
#include <QtWidgets/QMessageBox>

CongfigGui::CongfigGui(QWidget *parent, Qt::WindowFlags flags) : QWidget(parent, flags)
{
    ui = new Ui::ConfigGuiClass();
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setMouseTracking(true);

    ui->title->installEventFilter(this);
}

CongfigGui::~CongfigGui() = default;

bool CongfigGui::eventFilter(QObject *object, QEvent *event)
{
    static QPoint mousePoint;
    static bool   mousePressed = false;
    if (const auto *pEvent = dynamic_cast<QMouseEvent *>(event))
    {
        if (pEvent->type() == QEvent::MouseButtonPress)
        {
            if (pEvent->button() == Qt::LeftButton)
            {
                mousePressed = true;
                mousePoint   = pEvent->globalPos() - this->pos();
                return true;
            }
        }
        else if (pEvent->type() == QEvent::MouseButtonRelease)
        {
            mousePressed = false;
            return true;
        }
        else if (pEvent->type() == QEvent::MouseMove)
        {
            if (mousePressed && (pEvent->buttons() && Qt::LeftButton))
            {
                this->move(pEvent->globalPos() - mousePoint);
                return true;
            }
        }
    }

    return QWidget::eventFilter(object, event);
}

void CongfigGui::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    updateUI();
}

void CongfigGui::addLog(const char *log)
{
    /// 加入日期显示
    auto t = QTime::currentTime().toString("HH:mm:ss") + " " + log;
    LOGDEBUG(log);
    ui->log_list_Widget->insertItem(0, new QListWidgetItem(t));
}

void CongfigGui::updateUI()
{
    addLog("======开始刷新========");

    /// 清空表格
    while (ui->tableWidget->rowCount() > 0)
        ui->tableWidget->removeRow(0);

    /// 断开重连，如果修改配置中心的IP或者端口
    std::string       server_ip   = ui->server_ip_edit->text().toStdString();
    int               server_port = ui->server_port_box->value();
    std::stringstream ss;
    ss << server_ip << ":" << server_port;
    LOGDEBUG(ss.str().c_str());

    /// 关闭之前的连接，重新建立连接
    XConfigClient::get()->setServerIp(server_ip.c_str());
    XConfigClient::get()->setServerPort(server_port);
    XConfigClient::get()->setAutoDelete(false);
    XConfigClient::get()->close();
    if (!XConfigClient::get()->autoConnect(3))
    {
        addLog("无法连接到配置中心");
        return;
    }
    addLog("已成功连接到配置中心");

    /// 从配置中心获取配置列表
    const auto &config_list = XConfigClient::get()->getAllConfig(1, 10000, 10);
    LOGDEBUG(config_list.DebugString());

    /// 插入获取的列表

    ui->tableWidget->setRowCount(config_list.config_size());
    for (int i = 0; i < config_list.config_size(); i++)
    {
        auto config = config_list.config(i);
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(config.service_name().c_str()));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(config.service_ip().c_str()));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(config.service_port())));
    }
    addLog("======刷新完成========");
}

void CongfigGui::slotRefresh()
{
    updateUI();
}

void CongfigGui::slotAddConfig()
{
    /// 打开模态窗口，等待退出
    ConfigEdit edit;
    if (edit.exec() == QDialog::Accepted)
    {
        addLog("新增配置成功");
    }

    this->updateUI();
}

void CongfigGui::slotDeleteConfig()
{
    if (ui->tableWidget->rowCount() == 0)
        return;

    int row = ui->tableWidget->currentRow();
    if (row < 0)
        return;
    auto        item_name = ui->tableWidget->item(row, 0);
    auto        item_ip   = ui->tableWidget->item(row, 1);
    auto        item_port = ui->tableWidget->item(row, 2);
    std::string name      = item_name->text().toStdString();
    std::string ip        = item_ip->text().toStdString();
    int         port      = atoi(item_port->text().toStdString().c_str());

    std::stringstream ss;
    ss << "您确认删除" << name << "|" << ip << ":" << port << " 微服务配置吗？";
    if (QMessageBox::information(nullptr, "", ss.str().c_str(),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    {
        return;
    }
    XConfigClient::get()->deleteConfig(ip.c_str(), port);
    ss.clear();
    ss << "删除配置" << name << "|" << ip << ":" << port;
    addLog(ss.str().c_str());

    /// 获取选中的配置name IP port
    this->updateUI();
}

void CongfigGui::slotEditConfig()
{
    if (ui->tableWidget->rowCount() == 0)
        return;

    /// 获取需要编辑的配置ip和端口
    int row = ui->tableWidget->currentRow();
    if (row < 0)
        return;
    auto        item_ip   = ui->tableWidget->item(row, 1);
    auto        item_port = ui->tableWidget->item(row, 2);
    std::string ip        = item_ip->text().toStdString();
    int         port      = atoi(item_port->text().toStdString().c_str());

    /// 打开配置界面
    ConfigEdit edit;
    if (!edit.loadConfig(ip.c_str(), port))
    {
        addLog("读取配置失败!");
        return;
    }
    if (edit.exec() == QDialog::Accepted)
    {
        addLog("修改配置成功");
    }

    this->updateUI();
}
