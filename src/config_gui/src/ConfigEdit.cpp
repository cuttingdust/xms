﻿#include "ConfigEdit.h"
#include "ui_config_edit.h"

#include <XTools.h>
#include <XConfigClient.h>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QMessageBox>

#include <fstream>

static ConfigEdit *cur_edit = nullptr;

static void ConfigMessageCB(bool is_ok, const char *msg)
{
    if (cur_edit)
        cur_edit->signalMessageCB(is_ok, msg);
}

class ConfigEdit::PImpl
{
public:
    PImpl(ConfigEdit *owenr);
    ~PImpl() = default;

public:
    ConfigEdit                *owenr_            = nullptr;
    google::protobuf::Message *message_          = nullptr;
    xmsg::XConfig             *config_           = nullptr; /// 用于存储从配置中心获取的内容
    int                        config_row_count_ = 0;
};

ConfigEdit::PImpl::PImpl(ConfigEdit *owenr) : owenr_(owenr)
{
}

ConfigEdit::ConfigEdit(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
    cur_edit = this;

    ui = new Ui::ConfigEdit;
    ui->setupUi(this);

    impl_ = std::make_unique<PImpl>(this);

    connect(this, &ConfigEdit::signalMessageCB, this, &ConfigEdit::slotMessageCB);
    XConfigClient::get()->sendConfigResCB = ConfigMessageCB;
    impl_->config_row_count_              = ui->formLayout->rowCount();
}

ConfigEdit::~ConfigEdit()
{
    cur_edit = nullptr;
    delete impl_->config_;
    impl_->config_ = nullptr;
}

bool ConfigEdit::loadConfig(const char *ip, int port)
{
    /// 发送消息获取配置项 XConfig 存储到config_成员
    XConfigClient::get()->loadConfig(ip, port);
    if (!impl_->config_)
        impl_->config_ = new xmsg::XConfig();
    bool is_get = false;
    /// 等待一秒后超时
    for (int i = 0; i < 100; i++)
    {
        if (XConfigClient::get()->getConfig(ip, port, impl_->config_))
        {
            is_get = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!is_get)
    {
        LOGDEBUG("获取配置数据失败！");
        return false;
    }
    LOGDEBUG("获取配置数据成功！");
    if (impl_->config_->proto().empty())
    {
        LOGDEBUG("配置的proto为空！");
        return false;
    }

    /// 写入proto文件
    std::string   proto_filepath = "tmp.proto";
    std::ofstream ofs(proto_filepath);
    ofs << impl_->config_->proto();
    ofs.close();

    /// 加载proto 生成message
    std::string out_proto = "";
    /// 取proto文件中的第一个类型
    impl_->message_ = XConfigClient::get()->loadProto(proto_filepath, "", out_proto);
    if (!impl_->message_)
    {
        LOGDEBUG("加载proto文件失败！");
        return false;
    }

    //message反序列化，生成proto文件并加载

    //根据message 生成界面
    //根据message内容，写入到界面

    return true;
}

void ConfigEdit::slotSave()
{
    if (!impl_->message_)
    {
        LOGDEBUG("save failed! message is null");
        QMessageBox::information(this, "", "proto is empty");
        return;
    }

    if (ui->service_nameLineEdit->text().isEmpty() || ui->service_ipLineEdit->text().isEmpty() ||
        ui->proto_textEdit->toPlainText().isEmpty())
    {
        QMessageBox::information(this, "", "service_name,service_ip,proto is empty");
        return;
    }

    /// 将界面输入 存储到message中

    /// 配置信息
    /// 遍历输入
    /// 类型描述
    auto desc = impl_->message_->GetDescriptor();
    /// message 反射
    auto ref = impl_->message_->GetReflection();

    for (int i = impl_->config_row_count_; i < ui->formLayout->rowCount(); ++i)
    {
        //////////////////////////////////////////////
        ///找到key label的text
        auto label_item = ui->formLayout->itemAt(i, QFormLayout::LabelRole);
        if (!label_item)
            continue;
        /// 运行时转换，失败返回NULL
        auto label = dynamic_cast<QLabel *>(label_item->widget());
        if (!label)
            continue;
        auto field_name = label->text().toStdString();

        //////////////////////////////////////////////
        /// 获取value 获取输入控件中的值 枚举、整形、浮点、字符串
        /// 获取控件
        auto field_item = ui->formLayout->itemAt(i, QFormLayout::FieldRole);
        if (!field_item)
            continue;
        auto field_edit = field_item->widget();
        /// 获取字段描述（类型）
        auto field_desc = desc->FindFieldByName(field_name);
        if (!field_desc)
            continue;
        auto type = field_desc->type();

        /// 获取控件的值，设置到message

        QSpinBox       *int_box    = nullptr; /// 输入整形
        QDoubleSpinBox *double_box = nullptr; /// 输入浮点
        QLineEdit      *str_edit   = nullptr; /// 字符串 byte
        QComboBox      *combo_box  = nullptr; /// bool 枚举

        switch (type)
        {
                /// 支持整形数字
            case google::protobuf::FieldDescriptor::TYPE_INT64:
                int_box = dynamic_cast<QSpinBox *>(field_edit);
                if (int_box)
                    continue;
                ref->SetInt64(impl_->message_, field_desc, int_box->value());
                break;
            case google::protobuf::FieldDescriptor::TYPE_INT32:
                int_box = dynamic_cast<QSpinBox *>(field_edit);
                if (int_box)
                    continue;
                ref->SetInt32(impl_->message_, field_desc, double_box->value());
                break;
                /// 支持浮点数
            case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
                double_box = dynamic_cast<QDoubleSpinBox *>(field_edit);
                if (double_box)
                    continue;
                ref->SetDouble(impl_->message_, field_desc, double_box->value());
                break;
            case google::protobuf::FieldDescriptor::TYPE_FLOAT:
                double_box = dynamic_cast<QDoubleSpinBox *>(field_edit);
                if (double_box)
                    continue;
                ref->SetFloat(impl_->message_, field_desc, double_box->value());
                break;
            case google::protobuf::FieldDescriptor::TYPE_BYTES:
            case google::protobuf::FieldDescriptor::TYPE_STRING:
                str_edit = dynamic_cast<QLineEdit *>(field_edit);
                if (str_edit)
                    continue;
                ref->SetString(impl_->message_, field_desc, str_edit->text().toStdString());
                break;
            case google::protobuf::FieldDescriptor::TYPE_BOOL:
                combo_box = dynamic_cast<QComboBox *>(field_edit);
                if (combo_box)
                    continue;
                ref->SetBool(impl_->message_, field_desc, combo_box->currentData().toBool());
                break;
            case google::protobuf::FieldDescriptor::TYPE_ENUM:
                combo_box = dynamic_cast<QComboBox *>(field_edit);
                if (combo_box)
                    continue;
                ref->SetEnumValue(impl_->message_, field_desc, combo_box->currentData().toInt());
                break;
            default:
                break;
        }
    }

    /// 将界面输入 存储到message中
    /// 遍历界面 区分基础信息和配置信息
    xmsg::XConfig config;
    /// 基础信息
    config.set_service_name(ui->service_nameLineEdit->text().toStdString());
    config.set_service_ip(ui->service_ipLineEdit->text().toStdString());
    config.set_service_port(ui->service_portSpinBox->value());
    config.set_proto(ui->proto_textEdit->toPlainText().toStdString());
    /// 配置信息
    /// 序列化message
    std::string msg_pb = impl_->message_->SerializeAsString();
    config.set_private_pb(msg_pb);

    LOGDEBUG(impl_->message_->DebugString());
    LOGDEBUG(config.DebugString());
    /// 存储配置到配置中心

    XConfigClient::get()->sendConfig(&config);
}

void ConfigEdit::slotLoadProto()
{
    LOGDEBUG("LoadProto");
    /// 清理之前的配置项目
    ///
    while (ui->formLayout->rowCount() != impl_->config_row_count_)
        ui->formLayout->removeRow(impl_->config_row_count_);

    delete impl_->message_;
    impl_->message_ = nullptr;

    /// 用户输入类型名称，如果没有名称，则使用proto文件中的第一个类型
    QString     class_name = ui->proto_nameEdit->text();
    std::string class_name_str;
    if (!class_name.isEmpty())
    {
        class_name_str = class_name.toStdString();
    }

    /// 用户选择proto文件
    QString filename = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("请选择proto文件"), "", "*.proto");
    if (filename.isEmpty())
        return;
    LOGDEBUG(filename.toStdString().c_str());
    std::string proto_code;
    impl_->message_ = XConfigClient::get()->loadProto(filename.toStdString(), class_name_str, proto_code);
    if (impl_->message_ == nullptr)
    {
        LOGDEBUG("XConfigClient::Get()->LoadProto failed!");
        return;
    }
    ui->proto_nameEdit->setText(impl_->message_->GetTypeName().c_str());
    ui->proto_textEdit->setText(proto_code.c_str());

    /// 根据message 反射生成配置输入界面
    /// 获取类型描述
    auto desc = impl_->message_->GetDescriptor();
    /// 遍历字段
    int field_count = desc->field_count();
    for (int i = 0; i < field_count; i++)
    {
        /// 单个字段描述
        auto field = desc->field(i);

        /// 输入整形
        QSpinBox *int_box = nullptr;

        /// 输入浮点
        QDoubleSpinBox *double_box = nullptr;

        ///字符串 byte
        QLineEdit *str_edit = nullptr;

        /// bool 枚举
        QComboBox *combo_box = nullptr;

        /// 支持数字，字符串，枚举
        switch (const auto type = field->type())
        {
                ///支持整形数字
            case google::protobuf::FieldDescriptor::TYPE_INT32:
            case google::protobuf::FieldDescriptor::TYPE_INT64:
                // case google::protobuf::FieldDescriptor::TYPE_UINT64:
                // case google::protobuf::FieldDescriptor::TYPE_FIXED64:
                // case google::protobuf::FieldDescriptor::TYPE_FIXED32:
                // case google::protobuf::FieldDescriptor::TYPE_UINT32:
                // case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
                // case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
                // case google::protobuf::FieldDescriptor::TYPE_SINT32:
                // case google::protobuf::FieldDescriptor::TYPE_SINT64:
                int_box = new QSpinBox();
                int_box->setMaximum(INT32_MAX);
                ui->formLayout->addRow(field->name().c_str(), int_box);
                break;
                ///支持浮点数
            case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
            case google::protobuf::FieldDescriptor::TYPE_FLOAT:
                double_box = new QDoubleSpinBox();
                double_box->setMaximum(FLT_MAX);
                ui->formLayout->addRow(field->name().c_str(), double_box);
                break;
                /// 支持字符串
            case google::protobuf::FieldDescriptor::TYPE_BYTES:
            case google::protobuf::FieldDescriptor::TYPE_STRING:
                str_edit = new QLineEdit();
                ui->formLayout->addRow(field->name().c_str(), str_edit);
                break;
            case google::protobuf::FieldDescriptor::TYPE_BOOL:
                combo_box = new QComboBox();
                combo_box->addItem("true", true);
                combo_box->addItem("false", true);
                ui->formLayout->addRow(field->name().c_str(), combo_box);
                break;
            case google::protobuf::FieldDescriptor::TYPE_ENUM:
                combo_box = new QComboBox();
                for (int j = 0; j < field->enum_type()->value_count(); ++j)
                {
                    std::string enum_name = field->enum_type()->value(j)->name();
                    int         enum_val  = field->enum_type()->value(j)->number();
                    combo_box->addItem(enum_name.c_str(), enum_val);
                }
                ui->formLayout->addRow(field->name().c_str(), combo_box);
                break;

            case google::protobuf::FieldDescriptor::TYPE_GROUP:
                break;
            case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
                break;
            default:
                break;
        }
    }
}

void ConfigEdit::slotMessageCB(bool is_ok, const char *msg)
{
    if (!is_ok)
    {
        QMessageBox::information(this, "", msg);
        return;
    }

    accept();
}
