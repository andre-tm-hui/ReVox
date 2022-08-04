#include "fxdial.h"
#include "ui_fxdial.h"

FXDial::FXDial(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FXDial)
{
    ui->setupUi(this);

    connect(ui->dial, SIGNAL(valueChanged(int)), this, SLOT(onValueChange(int)));
}

FXDial::~FXDial()
{
    delete ui;
}

void FXDial::onValueChange(int value)
{
    ui->label->setText(QString::number((double)value, 'e', 2));
    emit valueChanged(value);
}
