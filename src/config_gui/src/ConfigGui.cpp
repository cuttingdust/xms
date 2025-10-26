#include "ConfigGui.h"
#include "ui_config_gui.h"

#include "ConfigEdit.h"
#include "XLoginGui.h"

#include <XTools.h>
#include <XConfigClient.h>
#include <XAuthClient.h>

#include <QtCore/QTime>
#include <QtGui/QtEvents>
#include <QtWidgets/QMessageBox>

static ConfigGui    *config_gui = nullptr;
static std::mutex    config_mtx;
static xmsg::XConfig config_tmp;

static void SGetConfigResCB(xmsg::XConfig config)
{
    XMutex mux(&config_mtx);
    config_tmp.CopyFrom(config);
    config_gui->SEditConfCB(&config_tmp);
}

ConfigGui::ConfigGui(QWidget *parent, Qt::WindowFlags flags) : QWidget(parent, flags)
{
    ui_ = new Ui::ConfigGuiClass();
    ui_->setupUi(this);
    config_gui = this;

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setMouseTracking(true);

    ui_->title->installEventFilter(this);

    connect(this, SIGNAL(SEditConfCB(void *)), this, SLOT(EditConfCB(void *)));

    //////////////////////////////////////////////////////////////////
    while (ui_->tableWidget->rowCount() > 0)
    {
        ui_->tableWidget->removeRow(0);
    }
    std::string server_ip   = ui_->server_ip_edit->text().toStdString();
    int         server_port = ui_->server_port_box->value();

    XConfigClient::get()->setServerIP(server_ip.c_str());
    XConfigClient::get()->setServerPort(server_port);
    XConfigClient::get()->setAutoConnect(true);
    XConfigClient::get()->startConnect();

    //////////////////////////////////////////////////////////////////


    auto auth_ip   = "127.0.0.1";
    auto auth_port = AUTH_PORT;
    XAuthClient::get()->setServerIP(auth_ip);
    XAuthClient::get()->setServerPort(auth_port);

    /// 设置自动重连
    XAuthClient::get()->setAutoConnect(true);
    XAuthClient::get()->startConnect();

    XConfigClient::get()->setLoadConfigCallBack(SGetConfigResCB);
    updateUI();
}

ConfigGui::~ConfigGui() = default;

bool ConfigGui::eventFilter(QObject *object, QEvent *event)
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

void ConfigGui::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    updateUI();
}

auto ConfigGui::addLog(const char *log) -> void
{
    /// 加入日期显示
    auto t = QTime::currentTime().toString("HH:mm:ss") + " " + log;
    LOGDEBUG(log);
    ui_->log_list_Widget->insertItem(0, new QListWidgetItem(t));
}

auto ConfigGui::checkLogin(std::string ip, int port) -> bool
{
    /// 验证登录是否有效，是否超时
    /// 超时前一分钟发送token延时命令
    static bool        is_login  = false;
    static std::string last_ip   = "";
    static int         last_port = 0;
    /// 要考虑更换服务器后的重新登录
    if (is_login && ip == last_ip && last_port == port)
    {
        return true;
    }

    last_ip   = ip;
    last_port = port;
    is_login  = false;
    XAuthClient::get()->setServerIP(ip.c_str());
    XAuthClient::get()->setServerPort(port);
    XAuthClient::get()->close();
    if (!XAuthClient::get()->autoConnect(1))
    {
        addLog("验证服务连接失败");
        return false;
    }

    XLoginGui login;
    if (login.exec() == QDialog::Rejected)
    {
        return false;
    }
    is_login = true;
    return true;
}

void ConfigGui::updateUI()
{
    addLog("======开始刷新========");

    /// 清空表格
    while (ui_->tableWidget->rowCount() > 0)
    {
        ui_->tableWidget->removeRow(0);
    }

    /// 断开重连，如果修改配置中心的IP或者端口
    std::string       server_ip   = ui_->server_ip_edit->text().toStdString();
    int               server_port = ui_->server_port_box->value();
    std::stringstream ss;
    ss << server_ip << ":" << server_port;
    LOGDEBUG(ss.str());

    if (!checkLogin(server_ip, server_port))
    {
        addLog("验证登录失败");
        return;
    }

    /// 清理历史列表
    addLog("清理历史列表");
    while (ui_->tableWidget->rowCount() > 0)
    {
        ui_->tableWidget->removeRow(0);
    }

    /// 关闭之前的连接，重新建立连接
    XConfigClient::get()->setServerIP(server_ip.c_str());
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

    ui_->tableWidget->setRowCount(config_list.config_size());
    for (int i = 0; i < config_list.config_size(); i++)
    {
        auto config = config_list.config(i);
        ui_->tableWidget->setItem(i, 0, new QTableWidgetItem(config.service_name().c_str()));
        ui_->tableWidget->setItem(i, 1, new QTableWidgetItem(config.service_ip().c_str()));
        ui_->tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(config.service_port())));
    }
    addLog("======刷新完成========");
}

void ConfigGui::slotRefresh()
{
    updateUI();
}

void ConfigGui::slotAddConfig()
{
    /// 打开模态窗口，等待退出
    ConfigEdit edit;
    if (edit.exec() == QDialog::Accepted)
    {
        addLog("新增配置成功");
    }

    this->updateUI();
}

void ConfigGui::slotDeleteConfig()
{
    if (ui_->tableWidget->rowCount() == 0)
    {
        return;
    }

    int row = ui_->tableWidget->currentRow();
    if (row < 0)
    {
        return;
    }
    auto        item_name = ui_->tableWidget->item(row, 0);
    auto        item_ip   = ui_->tableWidget->item(row, 1);
    auto        item_port = ui_->tableWidget->item(row, 2);
    std::string name      = item_name->text().toStdString();
    std::string ip        = item_ip->text().toStdString();
    int         port      = atoi(item_port->text().toStdString().c_str());

    std::stringstream ss;
    ss << "您确认删除" << name << "|" << ip << ":" << port << " 微服务配置吗？";
    if (QMessageBox::information(nullptr, "", ss.str().c_str(), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
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

void ConfigGui::slotEditConfig()
{
    if (ui_->tableWidget->rowCount() == 0)
    {
        return;
    }

    /// 获取需要编辑的配置ip和端口
    int row = ui_->tableWidget->currentRow();
    if (row < 0)
    {
        return;
    }
    auto item_ip   = ui_->tableWidget->item(row, 1);
    auto item_port = ui_->tableWidget->item(row, 2);
    if (!item_ip || !item_port)
    {
        return;
    }

    std::string ip   = item_ip->text().toStdString();
    int         port = atoi(item_port->text().toStdString().c_str());

    XConfigClient::get()->loadConfig(ip.c_str(), port);
}

void ConfigGui::EditConfCB(void *config)
{
    XMutex     mux(&config_mtx);
    ConfigEdit edit;
    edit.loadConfig(static_cast<xmsg::XConfig *>(config));
    //edit.LoadConfig(ip.c_str(), port);
    if (edit.exec() == QDialog::Accepted)
    {
        addLog("新增配置成功");
    }
    updateUI();
}
