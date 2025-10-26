/**
 * @file   FilePassword.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-24
 */

#ifndef FILEPASSWORD_H
#define FILEPASSWORD_H

#include <QtWidgets/QDialog>

class FilePassword : public QDialog
{
    Q_OBJECT
public:
    explicit FilePassword(QWidget* parent = Q_NULLPTR);
    ~FilePassword() override;

public:
    auto accept() -> void override;

    auto getPassWd() const -> std::string;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // FILEPASSWORD_H
