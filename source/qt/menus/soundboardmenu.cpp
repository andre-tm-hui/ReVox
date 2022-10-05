#include "soundboardmenu.h"
#include "ui_soundboardmenu.h"

SoundboardMenu::SoundboardMenu(json *hotkeys,
                               AudioManager *am,
                               KeyboardListener *kl,
                               QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SoundboardMenu)
{
    ui->setupUi(this);

    this->hotkeys = hotkeys;
    this->am = am;
    this->kl = kl;

    wv = new WaveformViewer(ui->waveform->size().width(),
                            ui->waveform->size().height(),
                            ui->waveform);
    ui->leftCover->raise();
    ui->rightCover->raise();

    // Setup selector
    connect(ui->hotkeysList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onHotkeySelect()));
    // Setup clip editor
    connect(ui->startAt, SIGNAL(sliderMoved(int)), this, SLOT(leftMarkerMoved(int)));
    connect(ui->startAt, SIGNAL(valueChanged(int)), this, SLOT(leftMarkerMoved(int)));
    connect(ui->endAt, SIGNAL(sliderMoved(int)), this, SLOT(rightMarkerMoved(int)));
    connect(ui->endAt, SIGNAL(valueChanged(int)), this, SLOT(rightMarkerMoved(int)));
    // Setup playback buttons
    connect(ui->play, SIGNAL(clicked()), this, SLOT(playSample()));
    connect(ui->stop, SIGNAL(clicked()), this, SLOT(stopSample()));
    // Setup hotkey settings
    connect(ui->recordMic, SIGNAL(stateChanged(int)), this, SLOT(setRecordMic()));
    connect(ui->recordSys, SIGNAL(stateChanged(int)), this, SLOT(setRecordSys()));
    connect(ui->overlap, SIGNAL(stateChanged(int)), this, SLOT(setOverlap()));
    connect(ui->loadClip, SIGNAL(clicked()), this, SLOT(loadClip()));
    // Setup slider
    connect(ui->slider, SIGNAL(sliderMoved(int)), this, SLOT(onSliderChanged(int)));
    connect(ui->slider, SIGNAL(valueChanged(int)), this, SLOT(onSliderChanged(int)));
    ui->slider->setValue(am->settings["monitorSamples"]);

    connect(ui->add, SIGNAL(clicked()), this, SLOT(addBind()));
    connect(ui->remove, SIGNAL(clicked()), this, SLOT(removeBind()));
    //ui->add->installEventFilter(keyPressEater);
    //ui->hotkeysLisit->installEventFilter(keyPressEater);

    for (auto& [keybind, settings] : (*hotkeys).items())
    {
        QString qKeybind = vkCodenames[std::stoi(keybind)];
        addBind(std::stoi(keybind), QString::fromStdString(settings["label"].get<std::string>()));
    }

    DisableSettings();
}

SoundboardMenu::~SoundboardMenu()
{
    delete ui;
}

void SoundboardMenu::DisableSettings()
{
    ui->settings->setEnabled(false);
    ui->editor->setEnabled(false);
    ui->hotkeysList->clearSelection();
    currHotkey = -1;
    wv->Clear();
    ui->overlayEditor->show();
    ui->overlaySettings->show();
    ui->remove->setEnabled(false);
}


// Slots
void SoundboardMenu::onHotkeySelect()
{
    HotkeyItem *item = qobject_cast<HotkeyItem *>(ui->hotkeysList->itemWidget(ui->hotkeysList->currentItem()));
    if (item->cb.keycode == currHotkey) return;
    QPropertyAnimation *a = fade(this, ui->settings, false, 200);
    QPropertyAnimation *b = fade(this, ui->editor, false, 200);
    fade(this, ui->overlayEditor, false, 200);
    fade(this, ui->overlaySettings, false, 200);
    connect(a, SIGNAL(finished()), this, SLOT(fadeInHotkey()));
}

void SoundboardMenu::fadeInHotkey()
{
    ui->settings->setEnabled(true);
    ui->editor->setEnabled(true);
    ui->overlayEditor->hide();
    ui->overlaySettings->hide();
    ui->remove->setEnabled(true);

    HotkeyItem *item = qobject_cast<HotkeyItem *>(ui->hotkeysList->itemWidget(ui->hotkeysList->currentItem()));
    currHotkey = item->cb.keycode;
    json settings = (*hotkeys)[std::to_string(currHotkey)];

    ui->recordMic->setCheckState(settings["recordInput"] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->recordSys->setCheckState(settings["recordLoopback"] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->overlap->setCheckState(settings["maxCopies"] <= 1 ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    wv->SetPath(am->GetAudioPath(currHotkey));
    wv->SetAudioClip();
    ui->startAt->setValue((int)(ui->startAt->maximum() * (int)settings["startAt"] / (float)wv->GetLength()));
    ui->endAt->setValue(((int)settings["endAt"] == -1) ? ui->endAt->maximum() : (int)(ui->endAt->maximum() * (int)settings["endAt"] / (float)wv->GetLength()));

    QPropertyAnimation *a = fade(this, ui->settings, true, 200);
    QPropertyAnimation *b = fade(this, ui->editor, true, 200);
    a->start(QPropertyAnimation::DeleteWhenStopped);
}

void SoundboardMenu::addBind(int keybind, QString label)
{
    HotkeyItem *item = new HotkeyItem(true,
                                      label == "" ? "Bind " + QString::number(ui->hotkeysList->count() + 1) : label,
                                      ui->hotkeysList->width(),
                                      keybind,
                                      am,
                                      kl);
    QListWidgetItem *orig = new QListWidgetItem();
    orig->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
    ui->hotkeysList->addItem(orig);
    ui->hotkeysList->setItemWidget(orig, item);
    orig->setSizeHint(QSize(0, 36));

    ui->hotkeysList->scrollToBottom();
    ui->hotkeysList->resize(ui->hotkeysList->width(), std::min(36 * ui->hotkeysList->count() + 2, 221));
}

void SoundboardMenu::removeBind()
{
    HotkeyItem *item = qobject_cast<HotkeyItem *>(ui->hotkeysList->itemWidget(ui->hotkeysList->currentItem()));
    am->RemoveBind(item->cb.keycode);
    ui->hotkeysList->removeItemWidget(ui->hotkeysList->currentItem());
    delete ui->hotkeysList->currentItem();
    DisableSettings();

    ui->hotkeysList->resize(ui->hotkeysList->width(), std::min(36 * ui->hotkeysList->count() + 2, 221));
}

void SoundboardMenu::setRecordMic()
{
    (*hotkeys)[std::to_string(currHotkey)]["recordInput"] = ui->recordMic->checkState() == Qt::CheckState::Checked;
    am->SaveBinds();
}

void SoundboardMenu::setRecordSys()
{
    (*hotkeys)[std::to_string(currHotkey)]["recordLoopback"] = ui->recordSys->checkState() == Qt::CheckState::Checked;
    am->SaveBinds();
}

void SoundboardMenu::setOverlap()
{
    (*hotkeys)[std::to_string(currHotkey)]["maxCopies"] = ui->overlap->checkState() == Qt::CheckState::Checked ? 999 : 1;
    am->SaveBinds();
}

void SoundboardMenu::loadClip()
{
    QString fname = QFileDialog::getOpenFileName(this, tr("Choose Soundbyte"), "/", tr("MPEG (*.mp3)"));
    if (std::filesystem::exists(fname.toStdString()))
    {
        am->OverrideSound(fname.toStdString(), currHotkey);
    }
    wv->SetPath(am->GetAudioPath(currHotkey));
    wv->SetAudioClip();
    am->soundboardHotkeys[std::to_string(currHotkey)]["startAt"] = 0;
    ui->startAt->setValue(0);
    am->soundboardHotkeys[std::to_string(currHotkey)]["endAt"] = wv->GetLength();
    ui->endAt->setValue(ui->endAt->maximum());
    am->SaveBinds();
}

void SoundboardMenu::leftMarkerMoved(int value)
{
    if (value >= ui->endAt->value())
    {
        value = ui->endAt->value();
        ui->startAt->setValue(value);
    }
    ui->leftCover->resize(value, ui->leftCover->height());

    am->soundboardHotkeys[std::to_string(currHotkey)]["startAt"] = (int)(wv->GetLength() * value / ui->startAt->maximum());
    am->SaveBinds();
}

void SoundboardMenu::rightMarkerMoved(int value)
{
    if (value <= ui->startAt->value())
    {
        value = ui->startAt->value();
        ui->endAt->setValue(value);
    }
    ui->rightCover->resize(ui->waveform->width() - value, ui->leftCover->height());
    ui->rightCover->move(value, ui->rightCover->y());

    am->soundboardHotkeys[std::to_string(currHotkey)]["endAt"] = (int)(wv->GetLength() * value / ui->endAt->maximum());
    am->SaveBinds();
}

void SoundboardMenu::playSample()
{
    am->Play(currHotkey, false);
}

void SoundboardMenu::stopSample()
{
    am->player->StopAll();
}

void SoundboardMenu::onSliderChanged(int value)
{
    am->SetSampleMonitor(value);
    ui->grooveOff->resize(ui->grooveOff->width(), 343 - 343 * value / 100);
}
