#include "keybindsettings.h"
#include "ui_keybindsettings.h"

/* Class for the keybind settings popup. */

KeybindSettings::KeybindSettings(keybind *settings, AudioManager *audioManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeybindSettings)
{
    ui->setupUi(this);
    this->samplerGroup = new QWidget(this);
    this->samplerGroup->move(0, 45);
    this->samplerGroup->setFixedSize(260, 125);
    ui->recordInput->setParent(this->samplerGroup);
    ui->recordLoopback->setParent(this->samplerGroup);
    ui->padAudio->setParent(this->samplerGroup);
    this->samplerGroup->hide();

    this->settings = settings;
    this->audioManager = audioManager;
    ui->recordInput->setCheckState(settings->recordInput ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->recordLoopback->setCheckState(settings->recordLoopback ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->padAudio->setCheckState(settings->padAudio ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    if (ui->padAudio->checkState() == Qt::CheckState::Checked)
    {
        ui->padAudio->setEnabled(true);
    }
    else
    {
        ui->padAudio->setEnabled(false);
    }

    this->fxGroup = new QWidget(this);
    this->fxGroup->move(0, 45);
    this->fxGroup->setFixedSize(260, 125);
    ui->fxType->setParent(this->fxGroup);
    ui->fxTypeLabel->setParent(this->fxGroup);
    this->fxGroup->hide();

    ui->fxType->setCurrentIndex(this->settings->fxType);

    this->reverbGroup = new QWidget(this->fxGroup);
    this->reverbGroup->move(0, 35);
    this->reverbGroup->setFixedSize(260, 90);
    fprintf(stdout, "%d, %d\n", this->reverbGroup->pos().x(), this->reverbGroup->pos().y()); fflush(stdout);
    ui->roomsize->setParent(this->reverbGroup);
    ui->roomsizeLabel->setParent(this->reverbGroup);
    ui->damp->setParent(this->reverbGroup);
    ui->dampLabel->setParent(this->reverbGroup);
    ui->width->setParent(this->reverbGroup);
    ui->widthLabel->setParent(this->reverbGroup);
    ui->wet->setParent(this->reverbGroup);
    ui->wetLabel->setParent(this->reverbGroup);
    ui->dry->setParent(this->reverbGroup);
    ui->dryLabel->setParent(this->reverbGroup);
    this->reverbGroup->hide();

    ui->roomsize->setValidator(new QDoubleValidator(0.0f, 1.0f, 1));
    ui->roomsize->setText(QString::number(this->settings->roomsize));
    ui->damp->setValidator(new QIntValidator(0, 100));
    ui->damp->setText(QString::number(this->settings->damp * 100));
    ui->width->setValidator(new QIntValidator(0, 100));
    ui->width->setText(QString::number(this->settings->width * 100));
    ui->wet->setValidator(new QIntValidator(0, 50));
    ui->wet->setText(QString::number(this->settings->wet * 100));
    ui->dry->setValidator(new QIntValidator(0, 50));
    ui->dry->setText(QString::number(this->settings->dry * 100));

    ui->hotkeyType->setCurrentIndex(this->settings->type);
    if (this->settings->type == 0)
    {
        this->samplerGroup->show();
        this->fxGroup->hide();
    }
    else if (this->settings->type == 1)
    {
        this->samplerGroup->hide();
        this->fxGroup->show();
        if (this->settings->fxType == 0)
        {
            this->reverbGroup->show();
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
        this->samplerGroup->show();
        this->fxGroup->hide();
    }
    else if (index == 1)
    {
        this->samplerGroup->hide();
        this->fxGroup->show();
        if (this->settings->fxType == 0)
        {
            this->reverbGroup->show();
        }
    }
}

