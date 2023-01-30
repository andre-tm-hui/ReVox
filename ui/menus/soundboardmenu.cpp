#include "soundboardmenu.h"
#include "ui_soundboardmenu.h"

SoundboardMenu::SoundboardMenu(std::shared_ptr<SoundboardManager> sm, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SoundboardMenu),
    sm(sm)
{
    ui->setupUi(this);

    crp = new Cropper(ui->cropperBG);
    crp->resize(ui->cropperBG->size());
    sm->SetWaveform(crp->getWaveformViewer());
    hkl = new HotkeyListWidget(ui->hotkeysContainer->size(), sm, "hotkeys", ui->hotkeysContainer);
    hkl->resize(ui->hotkeysContainer->size());
    modKey = new BindableButton(sm, "recordOver", false, ui->modifierPlaceholder);
    modKey->resize(ui->modifierPlaceholder->size());
    ui->monitorSlider->setValue(sm->GetSetting("monitor").get<int>());
    ui->monitorSlider->setLabel(":/rc/soundboardMenu/soundboardMonitorLabel.png");
    ui->padding->setValue(sm->GetSetting("padding").get<int>() / 1000.0);

    connect(hkl, SIGNAL(onSelectionChanged(int)), this, SLOT(setCurrentHotkey(int)));
    connect(hkl, SIGNAL(hotkeyRemoved()), this, SLOT(resetAll()));
    connect(crp, SIGNAL(onStartTimeChanged(int)), this, SLOT(setStartTime(int)));
    connect(crp, SIGNAL(onEndTimeChanged(int)), this, SLOT(setEndTime(int)));
    connect(crp, SIGNAL(onTransitionComplete(bool)), this, SLOT(toggleButtons(bool)));
    connect(ui->mic, SIGNAL(clicked(bool)), this, SLOT(setRecMic(bool)));
    connect(ui->system, SIGNAL(clicked(bool)), this, SLOT(setRecSystem(bool)));
    connect(ui->overlap, SIGNAL(clicked(bool)), this, SLOT(setOverlap(bool)));
    connect(ui->clipVolume, SIGNAL(valueChanged(int)), this, SLOT(setClipVolume(int)));
    connect(ui->load, SIGNAL(clicked(bool)), this, SLOT(loadFile()));
    connect(ui->del, SIGNAL(clicked(bool)), this, SLOT(delFile()));
    connect(ui->monitorSlider, SIGNAL(valueChanged(int)), this, SLOT(setMonitorVolume(int)));
    connect(ui->play, SIGNAL(clicked(bool)), this, SLOT(play()));
    connect(ui->stop, SIGNAL(clicked(bool)), this, SLOT(stop()));
    connect(ui->padding, SIGNAL(valueChanged(double)), this, SLOT(setPadding(double)));

    connect(hkl, &HotkeyListWidget::hotkeyAdded, this, [=]() { emit itemAdded(); });
    connect(crp, &Cropper::onClipChanged, this, [=]() { emit clipRecorded(); });

    resetAll();
}

SoundboardMenu::~SoundboardMenu()
{
    delete ui;
}

void SoundboardMenu::setCurrentHotkey(int val) {
    currHotkey = val;
    if (currHotkey == -1) {
        resetAll();
        return;
    }

    json hkSettings = sm->GetSetting("hotkeys/" + std::to_string(currHotkey));
    std::string clipPath = sm->GetRootDir() + "samples/" + std::to_string(currHotkey) + ".mp3";
    crp->setAudioPath(clipPath, hkSettings);
    ui->mic->setCheckState(hkSettings["recordInput"].get<bool>() ? Qt::Checked : Qt::Unchecked);
    ui->system->setCheckState(hkSettings["recordLoopback"].get<bool>() ? Qt::Checked : Qt::Unchecked);
    ui->overlap->setCheckState(hkSettings["maxCopies"].get<int>() == -1 ? Qt::Checked : Qt::Unchecked);
    ui->toggleGroup->setEnabled(true);
    ui->clipVolume->setValue(hkSettings["volume"].get<float>() * 100);
    ui->load->setEnabled(true);
    if (!std::filesystem::exists(clipPath)) {
        ui->play->setEnabled(false);
        ui->del->setEnabled(false);
        ui->clipVolume->setEnabled(false);
    } else {
        ui->play->setEnabled(true);
        ui->del->setEnabled(true);
        ui->clipVolume->setEnabled(true);
    }

    emit itemSelected();
}

void SoundboardMenu::resetAll() {
    currHotkey = -1;
    hkl->deselect();
    ui->mic->setCheckState(Qt::Unchecked);
    ui->system->setCheckState(Qt::Unchecked);
    ui->overlap->setCheckState(Qt::Unchecked);
    ui->toggleGroup->setEnabled(false);
    ui->clipVolume->setValue(100);
    ui->clipVolume->setEnabled(false);
    ui->play->setEnabled(false);
    crp->setAudioPath("", sm->GetDefaultHotkey());
    crp->setEnabled(false);
    ui->load->setEnabled(false);
    ui->del->setEnabled(false);
}

void SoundboardMenu::settingsEnabled(bool enabled) {
    ui->cropperBG->setEnabled(enabled);
    ui->hotkeysContainer->setEnabled(enabled);
}

void SoundboardMenu::setStartTime(int val) {
    if (currHotkey == -1) return;
    sm->UpdateSettings<int>("hotkeys/" + std::to_string(currHotkey) + "/startAt", val);
}

void SoundboardMenu::setEndTime(int val) {
    if (currHotkey == -1) return;
    sm->UpdateSettings<int>("hotkeys/" + std::to_string(currHotkey) + "/endAt", val);
}

void SoundboardMenu::setRecMic(bool val) {
    if (currHotkey == -1) return;
    sm->UpdateSettings<bool>("hotkeys/" + std::to_string(currHotkey) + "/recordInput", ui->mic->checkState() == Qt::CheckState::Checked ? true : false);
}

void SoundboardMenu::setRecSystem(bool val) {
    if (currHotkey == -1) return;
    sm->UpdateSettings<bool>("hotkeys/" + std::to_string(currHotkey) + "/recordLoopback", ui->system->checkState() == Qt::CheckState::Checked ? true : false);
}

void SoundboardMenu::setOverlap(bool val) {
    if (currHotkey == -1) return;
    sm->UpdateSettings<int>("hotkeys/" + std::to_string(currHotkey) + "/maxCopies", ui->overlap->checkState() == Qt::CheckState::Checked ? -1 : 1);
}

void SoundboardMenu::setClipVolume(int val) {
    if (currHotkey == -1) return;
    sm->UpdateSettings<float>("hotkeys/" + std::to_string(currHotkey) + "/volume", (float)val / 100);
}

void SoundboardMenu::setMonitorVolume(int val) {
    sm->SetMonitoringVol(val);
}

void SoundboardMenu::setPadding(double val) {
    sm->SetPadding((int)(val * 1000));
}

void SoundboardMenu::play() {
    if (currHotkey == -1) return;
    sm->Play(std::to_string(currHotkey));
}

void SoundboardMenu::stop() {
    sm->StopAll();
}

void SoundboardMenu::loadFile() {
    QString fname = QFileDialog::getOpenFileName(this, tr("Choose Soundbyte"), "/", tr("Audio Files (*.mp3 *.wav)"));
    if (std::filesystem::exists(fname.toStdString())) {
        sm->OverrideSound(fname.toStdString(), currHotkey);
    }
    std::string settingPath = "hotkeys/" + std::to_string(currHotkey);
    sm->UpdateSettings<int>(settingPath + "/startAt", 0);
    sm->UpdateSettings<int>(settingPath + "/endAt", -1);
    crp->setAudioPath(sm->GetRootDir() + "samples/" + std::to_string(currHotkey) + ".mp3", sm->GetSetting("hotkeys/" + std::to_string(currHotkey)));
    ui->play->setEnabled(true);
}

void SoundboardMenu::delFile() {
    std::string filePath = sm->GetRootDir() + "samples/" + std::to_string(currHotkey) + ".mp3";
    if (std::filesystem::exists(filePath)) {
        std::filesystem::remove(filePath);
        std::string settingPath = "hotkeys/" + std::to_string(currHotkey);
        sm->UpdateSettings<int>(settingPath + "/startAt", 0);
        sm->UpdateSettings<int>(settingPath + "/endAt", -1);
        crp->setAudioPath(filePath, sm->GetSetting(settingPath));
        toggleButtons(false);
    }
}

void SoundboardMenu::toggleButtons(bool enabled) {
    ui->play->setEnabled(enabled);
    ui->del->setEnabled(enabled);
    ui->clipVolume->setEnabled(enabled);
}
