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
    connect(ui->volume, SIGNAL(valueChanged(int)), this, SLOT(volumeChanged(int)));
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

    for (auto& [keybind, settings] : (*hotkeys).items())
    {
        addBind(settings["keycode"].get<int>(), QString::fromStdString(settings["label"].get<std::string>()));
    }

    ui->startAt->setMaximum(ui->waveform->width());
    ui->startAt->setValue(0);
    ui->endAt->setMaximum(ui->waveform->width());
    ui->endAt->setValue(ui->waveform->width());

    QString codename = "None";
    int rok = am->settings["recordOverKeybind"];
    if (rok >= 0 && rok < 128) codename = vkCodenames[rok];
    ui->recordOverKeybind->setText(codename);
    ui->recordOverKeybind->setToolTip(QString::fromStdString(am->settings["recordOverDeviceName"]));

    mapper = new QSignalMapper(this);
    connect(ui->recordOverKeybind, SIGNAL(clicked()), mapper, SLOT(map()));
    mapper->setMapping(ui->recordOverKeybind, (int)rebindType::rebind_recordOver);

    connect(mapper, SIGNAL(mappedInt(int)), this, SLOT(rebind(int)));
    connect(this, SIGNAL(newKeySet()), this, SLOT(cleanupRebind()), Qt::ConnectionType::QueuedConnection);

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
    emit itemSelected();
    HotkeyItem *item = qobject_cast<HotkeyItem *>(ui->hotkeysList->itemWidget(ui->hotkeysList->currentItem()));
    if (item->cb.idx == currHotkey) return;
    QPropertyAnimation *a = fade(this, ui->settings, false, 200);
    fade(this, ui->editor, false, 200);
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
    currHotkey = item->cb.idx;
    json settings = (*hotkeys)[std::to_string(currHotkey)];

    ui->recordMic->setCheckState(settings["recordInput"] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->recordSys->setCheckState(settings["recordLoopback"] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->overlap->setCheckState(settings["maxCopies"] <= 1 ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);

    changingClips = true;
    wv->SetPath(am->GetAudioPath(currHotkey));
    wv->SetAudioClip();
    ui->endAt->setValue(settings["endAt"].get<int>() == -1 ? ui->endAt->maximum() : (int)round(ui->endAt->maximum() * ((float)settings["endAt"] / (float)wv->GetLength())));
    ui->startAt->setValue((int)round(ui->startAt->maximum() * ((float)settings["startAt"] / (float)wv->GetLength())));
    ui->volume->setValue((int)(settings["volume"].get<float>() * 100));
    changingClips = false;

    QPropertyAnimation *a = fade(this, ui->settings, true, 200);
    fade(this, ui->editor, true, 200);
    a->start(QPropertyAnimation::DeleteWhenStopped);
}

void SoundboardMenu::addBind(int keybind, QString label)
{
    HotkeyItem *item = new HotkeyItem(true,
                                      label == "" ? "Bind " + QString::number(ui->hotkeysList->count() + 1) : label,
                                      ui->hotkeysList->width(),
                                      ui->hotkeysList->count(),
                                      keybind,
                                      am,
                                      this->parentWidget());
    QListWidgetItem *orig = new QListWidgetItem();
    orig->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
    ui->hotkeysList->addItem(orig);
    ui->hotkeysList->setItemWidget(orig, item);
    orig->setSizeHint(QSize(0, 36));

    ui->hotkeysList->scrollToBottom();
    ui->hotkeysList->resize(ui->hotkeysList->width(), __min(36 * ui->hotkeysList->count() + 2, 221));
    emit addBindPressed();
}

void SoundboardMenu::removeBind()
{
    HotkeyItem *item = qobject_cast<HotkeyItem *>(ui->hotkeysList->itemWidget(ui->hotkeysList->currentItem()));
    am->RemoveBind(item->cb.idx, bindType::soundboard);
    ui->hotkeysList->removeItemWidget(ui->hotkeysList->currentItem());
    delete ui->hotkeysList->currentItem();
    DisableSettings();

    for (int i = item->cb.idx; i < ui->hotkeysList->count(); i++) {
        HotkeyItem *item = qobject_cast<HotkeyItem *>(ui->hotkeysList->itemWidget(ui->hotkeysList->item(i)));
        item->cb.idx--;
    }

    ui->hotkeysList->resize(ui->hotkeysList->width(), __min(36 * ui->hotkeysList->count() + 2, 221));
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
    QString fname = QFileDialog::getOpenFileName(this, tr("Choose Soundbyte"), "/", tr("Audio Files (*.mp3 *.wav)"));
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
    if (!changingClips && value >= ui->endAt->value())
    {
        value = ui->endAt->value();
        ui->startAt->setValue(value);
    }
    ui->leftCover->resize(value, ui->leftCover->height());

    am->soundboardHotkeys[std::to_string(currHotkey)]["startAt"] = (int)(wv->GetLength() * (float)value / ui->startAt->maximum());
    am->SaveBinds();
}

void SoundboardMenu::rightMarkerMoved(int value)
{
    if (!changingClips && value <= ui->startAt->value())
    {
        value = ui->startAt->value();
        ui->endAt->setValue(value);
    }
    ui->rightCover->resize(ui->waveform->width() - value, ui->leftCover->height());
    ui->rightCover->move(value, ui->rightCover->y());

    am->soundboardHotkeys[std::to_string(currHotkey)]["endAt"] = (int)(wv->GetLength() * (float)value / ui->endAt->maximum());
    am->SaveBinds();
}

void SoundboardMenu::playSample()
{
    am->Play(std::to_string(currHotkey), false);
}

void SoundboardMenu::stopSample()
{
    am->player->StopAll();
}

void SoundboardMenu::volumeChanged(int value)
{
    am->soundboardHotkeys[std::to_string(currHotkey)]["volume"] = value / 100.f;
    am->SaveBinds();
}

void SoundboardMenu::onSliderChanged(int value)
{
    am->SetSampleMonitor(value);
    ui->grooveOff->resize(ui->grooveOff->width(), 343 - 343 * value / ui->slider->maximum());
}


void SoundboardMenu::rebind(int i)
{
    am->StartRebind();

    rbType = (rebindType)i;
    ui->recordOverKeybind->setText("Press a Key");
    ui->recordOverKeybind->setStyleSheet("QPushButton {border-radius: 3px; background-color: #303030; color: #AA0000;}"
                                         "QPushButton:hover {background-color: #404040;}");

    this->parentWidget()->setEnabled(false);

    std::thread t (WaitForKeyboardInput, this, am);
    t.detach();
}

void SoundboardMenu::WaitForKeyboardInput(void *widget, AudioManager *am)
{
    while (am->IsRebinding())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    SoundboardMenu *sm = (SoundboardMenu*)widget;
    emit sm->newKeySet();
}

void SoundboardMenu::cleanupRebind()
{
    int newKeycode = am->GetRebindDestination();
    QString qKeybind, qDeviceName;
    std::string keybindKey, dNameKey;
    switch (rbType) {
    case rebindType::rebind_recordOver:
        keybindKey = "recordOverKeybind";
        dNameKey = "recordOverDeviceName";
        break;
    }

    if (newKeycode == -1 && am->settings[keybindKey] == -1) newKeycode--;
    if (newKeycode == -1)
        newKeycode = am->settings[keybindKey];
    if (newKeycode >= 0) {
        qKeybind = vkCodenames[newKeycode];
        qDeviceName = QString::fromStdString(am->settings[dNameKey].get<std::string>());
    } else {
        qKeybind = "None";
        qDeviceName = "";
    }

    ui->recordOverKeybind->setText(qKeybind);
    ui->recordOverKeybind->setStyleSheet("QPushButton {border-radius: 3px; background-color: #303030; color: #FFFFFF;}"
                                         "QPushButton:hover {background-color: #404040;}");
    ui->recordOverKeybind->setToolTip(qDeviceName);

    am->SetNewBind(newKeycode, am->GetRebindDevice(), -1, bindType::recordOver);
    this->parentWidget()->setEnabled(true);
}
