#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QDialog>
#include <QBoxLayout>

class MessageDialog : public QDialog {
public:
    MessageDialog(QString title, bool includeLogo, QWidget* parent = nullptr);
    virtual ~MessageDialog();

    void addMainText(QString text);
    void addDefaultButtonBox();
    void centerOnParent(QWidget* parent);

    QVBoxLayout* mainVl;

protected slots:
    virtual void onAccepted();
    virtual void onRejected();
};

#endif // MESSAGEDIALOG_H