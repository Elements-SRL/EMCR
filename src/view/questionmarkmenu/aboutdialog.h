#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "messagedialog.h"

class AboutDialog : public MessageDialog {
    Q_OBJECT

public:
    AboutDialog(QWidget* parent = nullptr);

private:
    QString addLibrary(QString libName, QString libUrl, QString libVer, QString licenseName, QString licenseUrl);
    QString addLibrary(QString libName, QString libUrl, int libMaj, int libMin, int libPat, QString licenseName, QString licenseUrl);
};

#endif // ABOUTDIALOG_H
