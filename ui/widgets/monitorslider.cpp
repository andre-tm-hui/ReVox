#include "monitorslider.h"
#include "ui_monitorslider.h"

MonitorSlider::MonitorSlider(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MonitorSlider)
{
    ui->setupUi(this);
    connect(ui->monitor, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged(int)));
    connect(ui->monitor, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
}

MonitorSlider::~MonitorSlider()
{
    delete ui;
}

void MonitorSlider::setValue(int val) {
    ui->monitor->setValue(val);
}

void MonitorSlider::setLabel(QString url) {
    ui->label->setStyleSheet(QString("QWidget { image: url(\"%1\");}").arg(url));
}

void MonitorSlider::onValueChanged(int val) {
    ui->monitorOff->resize(
                ui->monitorOff->width(),
                edgeOffset + ((ui->monitorOn->height() - 2 * edgeOffset) * (ui->monitor->maximum() - val)) / ui->monitor->maximum()
                );
    emit valueChanged(val);
}
