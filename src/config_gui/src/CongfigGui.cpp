#include "CongfigGui.h"
#include "ui_config_gui.h"

#include <XTools.h>
#include <XConfigClient.h>

#include <QtGui/QtEvents>
#include <QtCore/QTime>

CongfigGui::CongfigGui(QWidget *parent, Qt::WindowFlags flags) : QWidget(parent, flags)
{
    ui = new Ui::ConfigGuiClass();
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    this->setMouseTracking(true);

    ui->title->installEventFilter(this);
}

CongfigGui::~CongfigGui() = default;

bool CongfigGui::eventFilter(QObject *object, QEvent *event)
{
    static QPoint mousePoint;
    static bool   mousePressed = false;
    const auto   *pEvent       = dynamic_cast<QMouseEvent *>(event);
    if (pEvent)
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

void CongfigGui::addLog(const char *log)
{
    /// 加入日期显示
    auto t = QTime::currentTime().toString("HH:mm:ss") + " " + QString::fromLocal8Bit(log);
    LOGDEBUG(log);
    ui->log_list_Widget->insertItem(0, new QListWidgetItem(t));
}

void CongfigGui::slotRefresh()
{
    addLog("Clean up the history list");

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
        addLog("Failed to connect to the configuration center");
        return;
    }
    addLog("Successfully connected to the configuration center");

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
    addLog("Update configuration list completed");
}
