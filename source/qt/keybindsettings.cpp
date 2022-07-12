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
    ui->recordLoopback->setCheckState(settings->recordLoopback ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->padAudio->setCheckState(settings->padAudio ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
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
    audioManager->SaveBinds();
}

