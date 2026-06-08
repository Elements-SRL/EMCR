#include "collapsiblesection.h"
#include <qstyle.h>

CollapsibleSection::CollapsibleSection(QString title, Qt::Orientation orientation) :
    QWidget(),
    title(title),
    orientation(orientation) {

    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QVBoxLayout * mainLo = new QVBoxLayout();
    this->setLayout(mainLo);
    mainLo->setContentsMargins(0, 0, 0, 0);
    mainLo->setSpacing(0);

    QHBoxLayout * titleLo = new QHBoxLayout();
    mainLo->addLayout(titleLo);

    hideShowBtn = new QToolButton();
    hideShowBtn->setCheckable(true);
    hideShowBtn->setChecked(false);

    titleEdit = new QLabel(title);
    titleEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QWidget * spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    titleLo->addWidget(hideShowBtn);
    titleLo->addWidget(titleEdit);
    titleLo->addWidget(spacer);

    valueLabel = new QLabel();
    valueLabel->setObjectName("collapsibleTitleValue");
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    valueLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    titleLo->addWidget(valueLabel);

    groupBox = new QGroupBox();
    groupBox->setObjectName("Collapsable");
    mainLo->addWidget(groupBox);
    groupBox->setVisible(false);

    if (orientation == Qt::Vertical) {
        gbLo = new QVBoxLayout();

    } else {
        gbLo = new QHBoxLayout();
    }
    groupBox->setLayout(gbLo);
    gbLo->setContentsMargins(3, 3, 3, 3);
    gbLo->setSpacing(3);

    buttonGroup = new QButtonGroup();

    connect(hideShowBtn, &QPushButton::clicked, this, &CollapsibleSection::onHideShowBtnClick);
    connect(buttonGroup, &QButtonGroup::buttonToggled, this, &CollapsibleSection::onButtonToggled);
}

void CollapsibleSection::setSingleOption(QString name) {
    QString leftText = name;
    QString rightText = "";

    int spaceIndex = name.lastIndexOf(" ");
    if (spaceIndex != -1) {
        leftText = name.left(spaceIndex).trimmed();
        rightText = name.mid(spaceIndex + 1).trimmed();
    }

    this->addRadioButton(name, true);

    titleEdit->setText(title);
    if (valueLabel) {
        valueLabel->setText(name);
    }

    hideShowBtn->setEnabled(false);
    singleOption = true;
    emit buttonToggled(0, true);
}

bool CollapsibleSection::getSingleOption() {
    return singleOption;
}

void CollapsibleSection::addLayout(QBoxLayout * lo) {
    gbLo->addLayout(lo);
}

void CollapsibleSection::addLayout(QGridLayout * lo) {
    gbLo->addLayout(lo);
}

void CollapsibleSection::addPushButton(QPushButton * btn, bool checked) {
    buttonGroup->addButton(btn, btnCount++);
    gbLo->addWidget(btn);
    if (checked == btn->isChecked()) {
        this->onButtonToggled(btn, checked);
        /*! The button emits the toggled signal only if it toggles, so to force the signal emission */
    }
    btn->setChecked(checked);
    buttons.push_back(btn);
}

void CollapsibleSection::addRadioButton(QString text, bool checked) {
    QString leftText = text;
    QString rightText = "";

    int spaceIndex = text.lastIndexOf(" ");
    if (spaceIndex != -1) {
        leftText = text.left(spaceIndex).trimmed(); // Ex: "100"
        rightText = text.mid(spaceIndex + 1).trimmed(); // Ex: "nA"
    }

    QRadioButton *rowButton = new QRadioButton();
    rowButton->setObjectName("collapsibleRowButton");
    rowButton->setProperty("fullText", text);

    QHBoxLayout *rowLayout = new QHBoxLayout(rowButton);
    rowLayout->setContentsMargins(8, 6, 8, 6);
    rowLayout->setSpacing(10);

    // Left label. Value only ex. "100"
    QLabel *lblLeft = new QLabel(leftText);
    lblLeft->setObjectName("rowLabelText");
    lblLeft->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // Right label. unit measure e.g "nA" etc
    QLabel *lblRight = new QLabel(rightText);
    lblRight->setObjectName("rowLabelValue");
    lblRight->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    rowLayout->addWidget(lblLeft, 1);
    rowLayout->addWidget(lblRight, 0);

    // Preventing labels to block the click on radio buttons below
    lblLeft->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblRight->setAttribute(Qt::WA_TransparentForMouseEvents);

    if (gbLo) {
        gbLo->addWidget(rowButton);
    }

    buttonGroup->addButton(rowButton, btnCount++);

    this->buttons.push_back(rowButton);

    if (checked == rowButton->isChecked()) {
        this->onButtonToggled(rowButton, checked);
    }
    rowButton->setChecked(checked);

    connect(rowButton, &QRadioButton::toggled, this, [rowButton](bool) {
        rowButton->style()->unpolish(rowButton);
        rowButton->style()->polish(rowButton);
    });
}

void CollapsibleSection::setParametricTitle(bool flag) {
    parametricTitleFlag = flag;
    if (flag) {
        /*! If title can contain selected control, controls must be exclusive */
        /*! \todo FCON c'era un motivo per il quale si può solo settare true il controllo esclusivo ma non tornare indietro, ma non me lo ricordo */
        this->setExclusiveControls(flag);
    }
}

void CollapsibleSection::setExclusiveControls(bool flag) {
    buttonGroup->setExclusive(flag);
    if (!flag) {
        /*! Title cannot contain selected control if controls are not exclusive */
        this->setParametricTitle(flag);
    }
}

void CollapsibleSection::onClickButton(int idx) {
    if (idx < btnCount) {
        buttonGroup->button(idx)->click();
        if (buttonGroup->button(idx)->isChecked()) {
            currentSelectedButton = idx;
        }
    }
}

void CollapsibleSection::onCheckButton(int idx, bool flag) {
    if (idx < btnCount) {
        if (flag) {
            currentSelectedButton = idx;
        }
        buttonGroup->button(idx)->setChecked(flag);
    }
}

void CollapsibleSection::onSetEnabled(bool flag) {
    if (!singleOption) {
        this->setEnabled(flag);
    }
}

void CollapsibleSection::addClosingSpacer() {
    QSpacerItem * w;
    if (orientation == Qt::Vertical) {
        w = new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);

    } else {
        w = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    gbLo->addSpacerItem(w);
}

void CollapsibleSection::setAvailable(bool flag, QString toolTip) {
    if (flag) {
        this->setStyleSheet(CLS_AVAILABLE_STYLESHEET);
        this->setToolTip("");

    } else {
        this->setStyleSheet(CLS_UNAVAILABLE_STYLESHEET);
        this->setToolTip(toolTip);
    }
    this->setVisible(flag);
}

int CollapsibleSection::getCurrentSelectedButton() {
    return currentSelectedButton;
}

std::vector <QAbstractButton *> CollapsibleSection::getButtons() {
    return buttons;
}

void CollapsibleSection::onHideShowBtnClick(bool show) {
    groupBox->setVisible(show);
}

void CollapsibleSection::onButtonToggled(QAbstractButton * btn, bool checked) {
    int idx = buttonGroup->id(btn);
    if (checked) {
        currentSelectedButton = idx;
    }
    emit buttonToggled(idx, checked);

    if (checked && parametricTitleFlag) {
        // (es: "100 nA")
        QString displayText = btn->property("fullText").toString();
        if (displayText.isEmpty()) {
            displayText = btn->text();
        }

        // Value only
        titleEdit->setText(title);
        if (valueLabel) {
            valueLabel->setText(displayText);
        }
    }
}
