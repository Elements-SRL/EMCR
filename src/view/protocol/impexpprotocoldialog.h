#ifndef IMPEXPPROTOCOLDIALOG_H
#define IMPEXPPROTOCOLDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QGridLayout>

#include "epmlmanager.h"
#include "globaldefines.h"
#include "e4gcommlib_global.h"

class ImpExpProtocolsActionSelector;

class ImpExpProtocolDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImpExpProtocolDialog(int clampingModality, QWidget * parent = nullptr);
    virtual ~ImpExpProtocolDialog();

    void show();
    QString getWriteInfo(QVector <bool> &saveFlag, QVector <bool> &overwriteFlag,
                         QStringList &names, QStringList &namesSet);

public slots:
    void onCheckAcceptability();

protected slots:
    virtual void onBrowse() = 0;
    void onSelectAllCbox(bool selected);
    void onCheckSelectAllCbox();

protected:
    QCheckBox * selectAllCbox;
    QDialogButtonBox * buttonBox;
    QLabel * fileNameLbl;
    QLineEdit * fileNameEdit;
    QLabel * titleLbl;
    QGridLayout * protocolManagementLo;
    QPushButton * browseBtn;

    int clampingModality;
    QString stimulusName;
    QString fullFileName;
    QString folderName;
    std::vector <std::string> sourceProtocolsNames;
    std::vector <std::string> destinationProtocolsNames;
    int protocolsNum = 0;
    QVector <ImpExpProtocolsActionSelector *> * actionSelectors = nullptr;
    EpmlManager * epmlManager = nullptr;

    virtual void checkFileContent() = 0;
};

class ExportProtocolDialog : public ImpExpProtocolDialog {
    Q_OBJECT

public:
    ExportProtocolDialog(QStringList pn, int clampingModality, QWidget * parent = nullptr);
    virtual ~ExportProtocolDialog();

protected slots:
    virtual void onBrowse() override;

protected:
    virtual void checkFileContent() override;
};

class ImportProtocolDialog : public ImpExpProtocolDialog {
    Q_OBJECT

public:
    ImportProtocolDialog(QStringList pn, int clampingModality, QWidget * parent = nullptr);
    virtual ~ImportProtocolDialog();

protected slots:
    virtual void onBrowse() override;

protected:
    virtual void checkFileContent() override;
};

class ImpExpProtocolsActionSelector : public QObject {
    Q_OBJECT

public:
    enum {
        ComboBoxNoSelection = 0,
        ComboBoxDontSave = 1,
        ComboBoxOverwrite = 2,
        ComboBoxRename = 3
    } CbBoxIdx_t;

    ImpExpProtocolsActionSelector();

    void setText(QString text);
    void setNameConflict(bool b);

    QCheckBox * getChBox();
    QLabel * getErrorLbl();
    QComboBox * getCbBox();
    QLineEdit * getLnEdit();

    QString getNameSet();
    bool acceptable();
    void setChecked(bool checked);
    bool isChecked();
    bool toBeSaved();
    bool overwrite();

protected:
    QCheckBox * chBox;
    QLabel * errorLbl;
    QComboBox * cbBox;
    QLineEdit * lnEdit;

    bool nameConflict = false;

protected slots:
    void onComboBoxChanged(int idx);

signals:
    void checkAcceptability();
    void checkSelectAllCbox();
};

class ExportProtocolsActionSelector : public ImpExpProtocolsActionSelector {
    Q_OBJECT

public:
    ExportProtocolsActionSelector();
};

class ImportProtocolsActionSelector : public ImpExpProtocolsActionSelector {
    Q_OBJECT

public:
    ImportProtocolsActionSelector();
};

#endif // IMPEXPPROTOCOLDIALOG_H
