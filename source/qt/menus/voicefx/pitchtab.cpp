#include "pitchtab.h"
#include "ui_pitchtab.h"

PitchTab::PitchTab(AudioManager *am, QTabWidget *tabWidget, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PitchTab)
{
    ui->setupUi(this);

    this->am = am;
    this->tabWidget = tabWidget;

    dial = new CustomDial(ui->offsetDial);
    dial->resize(46, 46);
    dial->setStyleSheet("background-color: #707070; color: #00FFFF; qproperty-knobMargin: 20px;");
    dial->setRange(0, 24);
    dial->setValue(12);

    connect(ui->toggle, SIGNAL(stateChanged(int)), this, SLOT(toggle(int)));
    connect(dial, SIGNAL(valueChanged(int)), this, SLOT(updateOffset(int)));
    connect(dial, SIGNAL(sliderReleased()), this, SLOT(releaseOffset()));
}

PitchTab::~PitchTab()
{
    delete ui;
}

void PitchTab::SetValues(json *values, bool animate)
{
    this->values = values;
    this->blockSignals(true);
    ui->toggle->setCheckState((*values)["enabled"] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    this->blockSignals(false);

    if (*this->values == nullptr) return;

    setFields(animate);
}

void PitchTab::setFields(bool animate)
{
    int val = 12;
    bool enabled = false;
    if ((*this->values)["enabled"])
    {
        val = (int)round(log((*this->values)["pitch"].get<float>()) / log(pow(2.f, 1.f / 12.f))) + 12;
        enabled = true;
    }

    if (animate)
    {
        dial->lerpToValue(val);
    }
    else
    {
        dial->setValue(val);
    }

    am->passthrough->data.pitchShift->setPitchscale((*this->values)["pitch"].get<float>());

    ui->settings->setEnabled(enabled);
}

void PitchTab::toggle(int state)
{
    if (state == 0)
    {
        tabWidget->setTabIcon(2, QIcon(":/icons/FXOff.png"));
        am->passthrough->data.pitchShift->setPitchshift(false);
        if (this->values != nullptr) (*this->values)["enabled"] = false;
        setFields();
        ui->overlay->show();
    }
    else
    {
        tabWidget->setTabIcon(2, QIcon(":/icons/FXOn.png"));
        am->passthrough->data.pitchShift->setPitchshift(true);
        if (this->values != nullptr) (*this->values)["enabled"] = true;
        setFields();
        ui->overlay->hide();
    }
    am->SaveBinds();
}

void PitchTab::releaseOffset()
{
    float value = pow(2, (dial->value() - 12) / 12.f);
    if (this->values != nullptr) (*this->values)["pitch"] = value;
    am->passthrough->data.pitchShift->setPitchscale(value);
    am->SaveBinds();
}

void PitchTab::updateOffset(int value)
{
    ui->lcdNumber->display(value - 12);
}
