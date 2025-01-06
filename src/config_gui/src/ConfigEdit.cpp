#include "ConfigEdit.h"
#include "ui_config_edit.h"

#include <XTools.h>
#include <XConfigClient.h>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QComboBox>

ConfigEdit::ConfigEdit(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
    ui = new Ui::ConfigEdit;
    ui->setupUi(this);
}

ConfigEdit::~ConfigEdit() = default;

void ConfigEdit::slotSave()
{
    accept();
}

void ConfigEdit::slotLoadProto()
{
    LOGDEBUG("LoadProto");

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
    auto        message = XConfigClient::get()->loadProto(filename.toStdString(), class_name_str, proto_code);
    if (message == nullptr)
    {
        LOGDEBUG("XConfigClient::Get()->LoadProto failed!");
        return;
    }
    ui->proto_nameEdit->setText(message->GetTypeName().c_str());
    ui->proto_textEdit->setText(proto_code.c_str());

    /// 根据message 反射生成配置输入界面
    /// 获取类型描述
    auto desc = message->GetDescriptor();
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
            case google::protobuf::FieldDescriptor::TYPE_INT64:
            case google::protobuf::FieldDescriptor::TYPE_UINT64:
            case google::protobuf::FieldDescriptor::TYPE_INT32:
            case google::protobuf::FieldDescriptor::TYPE_FIXED64:
            case google::protobuf::FieldDescriptor::TYPE_FIXED32:
            case google::protobuf::FieldDescriptor::TYPE_UINT32:
            case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
            case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
            case google::protobuf::FieldDescriptor::TYPE_SINT32:
            case google::protobuf::FieldDescriptor::TYPE_SINT64:
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
