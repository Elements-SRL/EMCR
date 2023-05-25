#ifndef ADDTAGDIALOG_H
#define ADDTAGDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>

class AddTagDialog : public QDialog {
public:
    AddTagDialog();

    QString getTag();

public slots:
    void open() override;

private:
    QLineEdit * tagEdit;
};

#endif // ADDTAGDIALOG_H
