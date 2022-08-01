#include "keybindsettings.h"
#include "ui_keybindsettings.h"

/* Class for the keybind settings popup. */

KeybindSettings::KeybindSettings(json *settings, AudioManager *audioManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeybindSettings)
{
    ui->setupUi(this);

    this->settings = settings;
    this->audioManager = audioManager;
    ui->recordInput->setCheckState((*settings)["recordInput"] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->recordLoopback->setCheckState((*settings)["recordLoopback"] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->padAudio->setCheckState((*settings)["syncStreams"] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    if (ui->padAudio->checkState() == Qt::CheckState::Checked)
    {
        ui->padAudio->setEnabled(true);
    }
    else
    {
        ui->padAudio->setEnabled(false);
    }
}

KeybindSettings::~KeybindSettings()
{
    delete ui;
}

/* On close, apply and save any changes */
void KeybindSettings::on_buttonBox_accepted()
{
    (*settings)["recordInput"] = ui->recordInput->isChecked();
    (*settings)["recordLoopback"] = ui->recordLoopback->isChecked();
    (*settings)["syncStreams"] = ui->padAudio->isChecked();
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



