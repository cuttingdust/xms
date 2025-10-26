#include "FilePassword.h"
#include "ui_file_password.h"
class FilePassword::PImpl
{
public:
    PImpl(FilePassword *owenr);
    ~PImpl() = default;

public:
    FilePassword     *owenr_   = nullptr;
    Ui::FilePassword *ui_      = nullptr;
    std::string       password = "";
};

FilePassword::PImpl::PImpl(FilePassword *owenr) : owenr_(owenr)
{
    ui_ = new Ui::FilePassword;
    ui_->setupUi(owenr_);
}

FilePassword::FilePassword(QWidget *parent) : QDialog(parent)
{
    impl_ = std::make_unique<FilePassword::PImpl>(this);
}

FilePassword::~FilePassword() = default;

auto FilePassword::accept() -> void
{
    impl_->password = impl_->ui_->passwordEdit->text().toStdString();
    QDialog::accept();
}

auto FilePassword::getPassWd() const -> std::string
{
    return impl_->password;
}
