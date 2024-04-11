#ifndef SUPPORTDIALOG_H
#define SUPPORTDIALOG_H

#include "messagedialog.h"

class SupportDialog : public MessageDialog {
    Q_OBJECT

public:
    SupportDialog(QWidget* parent = nullptr);
    ~SupportDialog();
};

#endif // SUPPORTDIALOG_H