#include "keybindsettings.h"
#include "ui_keybindsettings.h"

/* Class for the keybind settings popup. */

KeybindSettings::KeybindSettings(keybind *settings, AudioManager *audioManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeybindSettings)
{
    ui->setupUi(this);
    this->settings = settings;
    this->audioManager = audioManager;
    ui->recordInput->setCheckState(settings->recordInput ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->recordInput->hide();
    ui->recordLoopback->setCheckState(settings->recordLoopback ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->recordLoopback->hide();
    ui->padAudio->setCheckState(settings->padAudio ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    if (ui->padAudio->checkState() == Qt::CheckState::Checked)
    {
        ui->padAudio->setEnabled(true);
    }
    else
    {
        ui->padAudio->setEnabled(false);
    }
    ui->padAudio->hide();

    ui->fxType->setCurrentIndex(this->settings->fxType);
    ui->fxType->hide();
    ui->fxTypeLabel->hide();
    ui->roomsize->setValidator(new QDoubleValidator(0.0f, 1.0f, 2));
    ui->roomsize->setText(QString::number(this->settings->roomsize));
    ui->roomsize->hide();
    ui->roomsizeLabel->hide();
    ui->damp->setValidator(new QIntValidator(0, 100));
    ui->damp->setText(QString::number(this->settings->damp * 100));
    ui->damp->hide();
    ui->dampLabel->hide();
    ui->width->setValidator(new QIntValidator(0, 100));
    ui->width->setText(QString::number(this->settings->width * 100));
    ui->width->hide();
    ui->widthLabel->hide();
    ui->wet->setValidator(new QIntValidator(0, 200));
    ui->wet->setText(QString::number(this->settings->wet * 100));
    ui->wet->hide();
    ui->wetLabel->hide();
    ui->dry->setValidator(new QIntValidator(0, 200));
    ui->dry->setText(QString::number(this->settings->dry * 100));
    ui->dry->hide();
    ui->dryLabel->hide();

    ui->hotkeyType->setCurrentIndex(this->settings->type);
    if (this->settings->type == 0)
    {
        ui->recordInput->show();
        ui->recordLoopback->show();
        ui->padAudio->show();
    }
    else if (this->settings->type == 1)
    {
        ui->fxType->show();
        ui->fxTypeLabel->show();
        if (this->settings->fxType == 0)
        {
            ui->roomsize->show();
            ui->roomsizeLabel->show();
            ui->damp->show();
            ui->dampLabel->show();
            ui->width->show();
            ui->widthLabel->show();
            ui->wet->show();
            ui->wetLabel->show();
            ui->dry->show();
            ui->dryLabel->show();
        }
    }
}

KeybindSettings::~KeybindSettings()
{
    delete ui;
}

/* On close, apply and save any changes */
void KeybindSettings::on_buttonBox_accepted()
{
    settings->recordInput = ui->recordInput->isChecked();
    settings->recordLoopback = ui->recordLoopback->isChecked();
    settings->padAudio = ui->padAudio->isChecked();
    settings->type = ui->hotkeyType->currentIndex();
    settings->fxType = ui->fxType->currentIndex();
    settings->roomsize = ui->roomsize->text().toFloat();
    audioManager->passthrough->data.reverb->setroomsize(settings->roomsize);
    settings->damp = ui->damp->text().toFloat() / 100.0f;
    audioManager->passthrough->data.reverb->setdamp(settings->damp);
    settings->width = ui->width->text().toFloat() / 100.0f;
    audioManager->passthrough->data.reverb->setwidth(settings->width);
    settings->wet = ui->wet->text().toFloat() / 100.0f;
    audioManager->passthrough->data.reverb->setwet(settings->wet);
    settings->dry = ui->dry->text().toFloat() / 100.0f;
    audioManager->passthrough->data.reverb->setdry(settings->dry);
    audioManager->SaveBinds();
}


void KeybindSettings::on_recordLoopback_stateChanged(int arg1)
{
    if (arg1 == Qt::CheckState::Checked)
    {
        ui->padAudio->setEnabled(true);
    }
    else
    {
        ui->padAudio->setEnabled(false);
    }
}


void KeybindSettings::on_hotkeyType_currentIndexChanged(int index)
{
    if (index == 0)
    {
        ui->recordInput->show();
        ui->recordLoopback->show();
        ui->padAudio->show();

        ui->fxType->hide();
        ui->fxTypeLabel->hide();
        if (this->settings->fxType == 0)
        {
            ui->roomsize->hide();
            ui->roomsizeLabel->hide();
            ui->damp->hide();
            ui->dampLabel->hide();
            ui->width->hide();
            ui->widthLabel->hide();
            ui->wet->hide();
            ui->wetLabel->hide();
            ui->dry->hide();
            ui->dryLabel->hide();
        }
    }
    else
    {
        ui->recordInput->hide();
        ui->recordLoopback->hide();
        ui->padAudio->hide();

        ui->fxType->show();
        ui->fxTypeLabel->show();
        if (this->settings->fxType == 0)
        {
            ui->roomsize->show();
            ui->roomsizeLabel->show();
            ui->damp->show();
            ui->dampLabel->show();
            ui->width->show();
            ui->widthLabel->show();
            ui->wet->show();
            ui->wetLabel->show();
            ui->dry->show();
            ui->dryLabel->show();
        }
    }
}

