#include "channeloverviewwidget.h"

ChannelOverviewWidget::ChannelOverviewWidget(QWidget * parent) :
    QFrame(parent) {

    mainVl = new QVBoxLayout;
    mainVl->setContentsMargins(0, 0, 0, 0);
    mainVl->setSpacing(0);

    this->setLayout(mainVl);

    channelLbl = new QLabel;
    mainVl->addWidget(channelLbl);

    rmsLbl = new QLabel;
    mainVl->addWidget(rmsLbl);

    this->setFrameStyle(QFrame::Box | QFrame::Plain);
    this->setLineWidth(1);
}

void ChannelOverviewWidget::setVisualizationOption(VisualizationOption_t type) {
    switch (type) {
    case Plot:
        channelLbl->setVisible(false);
        rmsLbl->setVisible(false);
        plot->setVisible(true);
        break;

    case Noise:
        channelLbl->setVisible(true);
        rmsLbl->setVisible(true);
        plot->setVisible(false);
        break;
    }
}

void ChannelOverviewWidget::setChannelIndex(int index) {
    channelLbl->setText(QString("%1").arg(index+1));
    channelLbl->setStyleSheet("QLabel {font-weight: bold;}");
}

void ChannelOverviewWidget::setStampPlot(StampPlot * plot) {
    this->plot = plot;
    mainVl->addWidget(plot);
    connect(plot, &StampPlot::clicked, this, &ChannelOverviewWidget::clicked);
}

void ChannelOverviewWidget::setNoiseValue(Measurement_t value) {
    rmsLbl->setText(QString().fromStdString(value.niceLabel()));
}
