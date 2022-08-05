#include "pitchsettings.h"
#include "ui_pitchsettings.h"

PitchSettings::PitchSettings(PitchShift *pitchshift, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PitchSettings)
{
    this->pitchshift = pitchshift;
    ui->setupUi(this);

    connect(ui->dial, SIGNAL(valueChanged(int)), this, SLOT(onChange(int)));
}

PitchSettings::~PitchSettings()
{
    delete ui;
}

void PitchSettings::onChange(int value)
{
    // -12 -> 0.5, 0 -> 1, 12 -> 2.0
    // semitoneOffset = log(ffreq / 440) / log(12throotof2)
    float scale = pow(2.f, value / 12.f);
    pitchshift->setPitchscale(scale);

    ui->label_2->setText(QString::number(value) + "st");
}
