#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "messagedialog.h"

class AboutDialog : public MessageDialog {
    Q_OBJECT

public:
    AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog();
};

#endif // ABOUTDIALOG_H