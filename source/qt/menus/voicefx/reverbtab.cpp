#include "reverbtab.h"
#include "ui_reverbtab.h"

ReverbTab::ReverbTab(AudioManager *am, QTabWidget *tabWidget, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReverbTab)
{
    ui->setupUi(this);

    this->am = am;
    this->tabWidget = tabWidget;

    wetdry = new CustomDial(ui->wetdryDial);
    roomsize = new CustomDial(ui->roomsizeDial);
    width = new CustomDial(ui->widthDial);
    damp = new CustomDial(ui->dampDial);

    wetdry->resize(46, 46);
    roomsize->resize(46, 46);
    width->resize(46, 46);
    damp->resize(46, 46);

    wetdry->setStyleSheet("background-color: #707070; color: #00FFFF; qproperty-knobMargin: 20px;");
    roomsize->setStyleSheet("background-color: #707070; color: #00FFFF; qproperty-knobMargin: 20px;");
    width->setStyleSheet("background-color: #707070; color: #00FFFF; qproperty-knobMargin: 20px;");
    damp->setStyleSheet("background-color: #707070; color: #00FFFF; qproperty-knobMargin: 20px;");

    connect(ui->toggle, SIGNAL(stateChanged(int)), this, SLOT(toggle(int)));
    connect(wetdry, SIGNAL(valueChanged(int)), this, SLOT(updateWetdry(int)));
    connect(roomsize, SIGNAL(valueChanged(int)), this, SLOT(updateRoomsize(int)));
    connect(width, SIGNAL(valueChanged(int)), this, SLOT(updateWidth(int)));
    connect(damp, SIGNAL(valueChanged(int)), this, SLOT(updateDamp(int)));

    connect(wetdry, SIGNAL(sliderReleased()), this, SLOT(releaseWetdry()));
    connect(roomsize, SIGNAL(sliderReleased()), this, SLOT(releaseRoomsize()));
    connect(width, SIGNAL(sliderReleased()), this, SLOT(releaseWidth()));
    connect(damp, SIGNAL(sliderReleased()), this, SLOT(releaseDamp()));
}

ReverbTab::~ReverbTab()
{
    delete ui;
}

void ReverbTab::SetValues(json *values, bool animate)
{
    if (values != nullptr)
    {
        this->values = values;
        ui->toggle->setCheckState((*this->values)["enabled"] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    }

    if (*this->values == nullptr) return;

    if ((*this->values)["enabled"])
    {
        if (animate)
        {
            wetdry->lerpToValue((int)(100 * (*this->values)["mix"].get<float>()));
            roomsize->lerpToValue((int)(100 * (*this->values)["roomsize"].get<float>()));
            width->lerpToValue((int)(100 * (*this->values)["width"].get<float>()));
            damp->lerpToValue((int)(100 * (*this->values)["damp"].get<float>()));
        }
        else
        {
            wetdry->setValue((int)(100 * (*this->values)["mix"].get<float>()));
            roomsize->setValue((int)(100 * (*this->values)["roomsize"].get<float>()));
            width->setValue((int)(100 * (*this->values)["width"].get<float>()));
            damp->setValue((int)(100 * (*this->values)["damp"].get<float>()));
        }

        ui->settings->setEnabled(true);
    }
    else
    {
        if (animate)
        {
            wetdry->lerpToValue(0);
            roomsize->lerpToValue(0);
            width->lerpToValue(0);
            damp->lerpToValue(0);
        }
        else
        {
            wetdry->setValue(0);
            roomsize->setValue(0);
            width->setValue(0);
            damp->setValue(0);
        }

        ui->settings->setEnabled(false);
    }
}

void ReverbTab::toggle(int state)
{
    if (state == 0)
    {
        tabWidget->setTabIcon(0, QIcon(":/icons/FXOff.png"));
        am->passthrough->data.reverb->setEnabled(false);
        if (this->values != nullptr) (*this->values)["enabled"] = false;
        SetValues(nullptr, true);
        ui->overlay->show();
    }
    else
    {
        tabWidget->setTabIcon(0, QIcon(":/icons/FXOn.png"));
        am->passthrough->data.reverb->setEnabled(true);
        if (this->values != nullptr) (*this->values)["enabled"] = true;
        SetValues(nullptr, true);
        ui->overlay->hide();
    }
    am->SaveBinds();
}

void ReverbTab::releaseWetdry()
{
    float value = wetdry->value() / 100.f;
    if (*this->values != nullptr) (*this->values)["mix"] = value;
    am->passthrough->data.reverb->setwet(value);
    am->passthrough->data.reverb->setdry(1.f - value);
    am->SaveBinds();
}

void ReverbTab::releaseRoomsize()
{
    float value = roomsize->value() / 100.f;
    if (*this->values != nullptr) (*this->values)["roomsize"] = value;
    am->passthrough->data.reverb->setroomsize(value);
    am->SaveBinds();
}

void ReverbTab::releaseWidth()
{
    float value = width->value() / 100.f;
    if (*this->values != nullptr) (*this->values)["width"] = value;
    am->passthrough->data.reverb->setwidth(value);
    am->SaveBinds();
}

void ReverbTab::releaseDamp()
{
    float value = damp->value() / 100.f;
    if (*this->values != nullptr) (*this->values)["damp"] = value;
    am->passthrough->data.reverb->setdamp(value);
    am->SaveBinds();
}

void ReverbTab::updateWetdry(int value)
{
    ui->wetdryVal->display(value);
}

void ReverbTab::updateRoomsize(int value)
{
    ui->roomsizeVal->display(value);
}

void ReverbTab::updateWidth(int value)
{
    ui->widthVal->display(value);
}

void ReverbTab::updateDamp(int value)
{
    ui->dampVal->display(value);
}
