#include "reverbsettings.h"
#include "ui_reverbsettings.h"

ReverbSettings::ReverbSettings(revmodel *reverb, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReverbSettings)
{
    ui->setupUi(this);
    this->reverb = reverb;
    ui->mix->setValue((int)(this->reverb->getwet() * 100));
    ui->roomsize->setValue((int)(this->reverb->getroomsize() * 100));
    ui->width->setValue((int)(this->reverb->getwidth() * 100));
    ui->damp->setValue((int)(this->reverb->getdamp() * 100));
}

ReverbSettings::~ReverbSettings()
{
    delete ui;
}

void ReverbSettings::on_mix_valueChanged(int value)
{
    ui->mixValue->setText(QString::number(((double)value / (double)100), 'g', 2));
    reverb->setwet(value / 100.f);
    reverb->setdry(1.f - (value / 100.f));
}


void ReverbSettings::on_roomsize_valueChanged(int value)
{
    ui->roomsizeValue->setText(QString::number(((double)value / (double)100), 'g', 2));
    reverb->setroomsize(value / 100.f);
}


void ReverbSettings::on_width_valueChanged(int value)
{
    ui->widthValue->setText(QString::number(((double)value / (double)100), 'g', 2));
    reverb->setwidth(value / 100.f);
}


void ReverbSettings::on_damp_valueChanged(int value)
{
    ui->dampValue->setText(QString::number(((double)value / (double)100), 'g', 2));
    reverb->setdamp(value / 100.f);
}

