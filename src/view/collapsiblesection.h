#ifndef COLLAPSIBLESECTION_H
#define COLLAPSIBLESECTION_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#include <QButtonGroup>
#include <QRadioButton>
#include <QBoxLayout>
#include <QPushButton>
#include <QSpinBox>

#define CLS_AVAILABLE_STYLESHEET QString("background-color: rgb(240, 240, 240)")
#define CLS_UNAVAILABLE_STYLESHEET QString("background-color: rgb(255, 128, 128)")

class CollapsibleSection : public QWidget {
    Q_OBJECT

public:
    CollapsibleSection(QString title = "", Qt::Orientation orientation = Qt::Vertical);

    void setSingleOption(QString name);
    bool getSingleOption();
    void addLayout(QBoxLayout * lo);
    void addLayout(QGridLayout * lo);
    void addPushButton(QPushButton * btn, bool checked = false);
    void addRadioButton(QString name, bool checked = false);
    void setParametricTitle(bool flag);
    void setExclusiveControls(bool flag = false);
    void addClosingSpacer();
    void setAvailable(bool flag, QString toolTip = "");
    int getCurrentSelectedButton();
    std::vector <QAbstractButton *> getButtons();

public slots:
    void onClickButton(int idx);
    void onCheckButton(int idx, bool checked);
    void onSetEnabled(bool flag);

private slots:
    void onHideShowBtnClick(bool show);
    void onButtonToggled(QAbstractButton * btn, bool checked);

signals:
    void buttonToggled(int index, bool flag);
    void valueChanged(int);

private:
    QLabel * titleEdit;
    QString title;
    Qt::Orientation orientation;
    QToolButton * hideShowBtn;
    QGroupBox * groupBox;
    QBoxLayout * gbLo;
    QButtonGroup * buttonGroup;
    int btnCount = 0;
    bool parametricTitleFlag = false;
    bool singleOption = false;
    int currentSelectedButton = 0;
    std::vector <QAbstractButton *> buttons;
};

#endif // COLLAPSIBLESECTION_H
