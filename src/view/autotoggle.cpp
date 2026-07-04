#include "autotoggle.h"
#include <QHBoxLayout>
#include <QStyle>

AutoToggle::AutoToggle(QWidget *parent) : QWidget(parent) {
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    m_switch = new ActivationButton(this);
    m_label = new QLabel("AUTO", this);
    m_label->setObjectName("autoLabel");

    layout->addWidget(m_switch);
    layout->addWidget(m_label);

    connect(m_switch, &ActivationButton::clicked, this, [=](bool checked) {
        this->setChecked(checked);
        emit toggled(checked);
    });
}

void AutoToggle::setChecked(bool checked) {
    m_switch->setChecked(checked);
    m_label->setProperty("active", checked);
    m_label->style()->unpolish(m_label);
    m_label->style()->polish(m_label);
}
