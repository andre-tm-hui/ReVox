#include "settingsmenu.h"

#include "ui_settingsmenu.h"

SettingsMenu::SettingsMenu(std::shared_ptr<MainInterface> mi, HUD *hud,
                           QWidget *parent)
    : QWidget(parent),
      LoggableObject("Qt-SettingsMenu"),
      ui(new Ui::SettingsMenu),
      mi(mi),
      hud(hud) {
  ui->setupUi(this);

  positionsCheckboxes.push_back(ui->topLeft);
  positionsCheckboxes.push_back(ui->top);
  positionsCheckboxes.push_back(ui->topRight);
  positionsCheckboxes.push_back(ui->left);
  positionsCheckboxes.push_back(ui->right);
  positionsCheckboxes.push_back(ui->bottomLeft);
  positionsCheckboxes.push_back(ui->bottom);
  positionsCheckboxes.push_back(ui->bottomRight);

  mapper = new QSignalMapper();

  for (int i = 0; i < 8; i++) {
    mapper->setMapping(positionsCheckboxes[i], i);
    connect(positionsCheckboxes[i], SIGNAL(stateChanged(int)), mapper,
            SLOT(map()));
  }

  int hudPosition = mi->GetSetting("hudPosition").get<int>();
  if (hudPosition != -1) {
    positionsCheckboxes[hudPosition]->setCheckState(Qt::CheckState::Checked);
    ui->hudPositionLabel->setText(positionLabels[hudPosition]);
  }

  connect(mapper, SIGNAL(mappedInt(int)), this, SLOT(hudPositionChanged(int)));

  if (hud->GetPosition() >= 0)
    positionsCheckboxes[hud->GetPosition()]->setCheckState(
        Qt::CheckState::Checked);

  connect(ui->devices, SIGNAL(currentIndexChanged(int)), this,
          SLOT(deviceChanged()));
  connect(ui->reset, SIGNAL(pressed()), this->parent(), SLOT(resetAudio()));

  setupSwitch(&keyboardDetectorSwitch, ui->detectKeyboardBox);
  keyboardDetectorSwitch->setChecked(
      mi->GetSetting("detectKeyboard").get<bool>());
  connect(keyboardDetectorSwitch, SIGNAL(stateChanged(int)), this,
          SLOT(toggleKeyboardDetector(int)));

  setupSwitch(&blockInputsSwitch, ui->blockInputsBox);
  blockInputsSwitch->setChecked(mi->GetSetting("blockInputs").get<bool>());
  connect(blockInputsSwitch, SIGNAL(stateChanged(int)), this,
          SLOT(toggleInputBlocker(int)));

  setupSwitch(&autostartSwitch, ui->autostartBox);
  autostartSwitch->setChecked(mi->GetSetting("startWithWindows").get<bool>());
  connect(autostartSwitch, SIGNAL(stateChanged(int)), this,
          SLOT(toggleAutostart(int)));

  connect(ui->update, SIGNAL(pressed()), this, SLOT(checkForUpdates()));

  setupSwitch(&autocheckSwitch, ui->updateBox, 35);
  autocheckSwitch->setChecked(mi->GetSetting("autocheckUpdates").get<bool>());
  connect(autocheckSwitch, SIGNAL(stateChanged(int)), this,
          SLOT(toggleAutocheck(int)));

  if (mi->GetSetting("autocheckUpdates").get<bool>()) {
    log(INFO, "Checking for updates");
    CUpdaterDialog *updater = new CUpdaterDialog(
        (QWidget *)this->parent(),
        "https://api.github.com/repos/andre-tm-hui/ReVox/releases/latest",
        VER_NO, true);
    updater->setStyleSheet("QTextEdit {border: 1px solid #009090;}");
  }

  int bufferSize = 1;
  for (int i = 1; i < 12; i++) {
    bufferSize *= 2;
    ui->bufferSizes->addItem(QString::number(bufferSize));
  }
  ui->bufferSizes->setCurrentText(
      QString::number(mi->GetSetting("framesPerBuffer").get<int>()));
  connect(ui->bufferSizes, SIGNAL(currentTextChanged(QString)), this,
          SLOT(setNewBufferSize(QString)));
  ui->bufferSizeBox->hide();

  connect(ui->restartTutorial, SIGNAL(pressed()), this->parentWidget(),
          SLOT(restartTutorial()));
  log(INFO, "SettingsMenu setup");
}

SettingsMenu::~SettingsMenu() { delete ui; }

void SettingsMenu::SetDevices(std::map<std::string, device> &devices,
                              int currentDevice) {
  this->devices = devices;
  this->currentDevice = currentDevice;
  populateDevices();
  log(INFO, "Device list populated");
}

void SettingsMenu::populateDevices() {
  ui->devices->blockSignals(true);
  ui->devices->clear();
  for (auto const &[name, device] : devices) {
      int id = device.ids.find("Windows WASAPI") == device.ids.end() ? device.ids.at("MME") : device.ids.at("Windows WASAPI");
    if (name.find("VB-Audio") != std::string::npos || Pa_GetDeviceInfo(id)->defaultSampleRate != 48000) continue;
    ui->devices->addItem(QString::fromStdString(name));
    if (id == currentDevice) {
      ui->devices->setCurrentIndex(ui->devices->count() - 1);
    }
  }
  ui->devices->blockSignals(false);
}

void SettingsMenu::deviceChanged() {
  for (auto const &[name, device] : devices) {
    if (QString::fromStdString(name) == ui->devices->currentText()) {
        int id = device.ids.find("Windows WASAPI") == device.ids.end() ? device.ids.at("MME") : device.ids.at("Windows WASAPI");
      mi->SetCurrentOutputDevice(currentDevice);
      log(INFO, name + " set as device");
      break;
    }
  }
}

void SettingsMenu::hudPositionChanged(int item) {
  for (int i = 0; i < 8; i++) {
    if (i != item) {
      positionsCheckboxes[i]->blockSignals(true);
      positionsCheckboxes[i]->setCheckState(Qt::CheckState::Unchecked);
      positionsCheckboxes[i]->blockSignals(false);
    }
  }

  QString hudPositionText = "";
  if (positionsCheckboxes[item]->checkState() == Qt::CheckState::Unchecked) {
    hudPositionText = "Off";
    hud->SetPosition(-1);
    mi->UpdateSettings<int>("hudPosition", -1);
  } else {
    hudPositionText = positionLabels[item];
    hud->SetPosition(item);
    mi->UpdateSettings<int>("hudPosition", item);
  }

  ui->hudPositionLabel->setText(hudPositionText);
}

void SettingsMenu::toggleKeyboardDetector(int state) {
  bool enabled = state == 0 ? true : false;
  mi->UpdateSettings<bool>("detectKeyboard", enabled);
  log(INFO,
      "Keyboard detection " + std::string(enabled ? "enabled" : "disabled"));
}

void SettingsMenu::toggleInputBlocker(int state) {
  bool enabled = state == 0 ? false : true;
  if (mi->ToggleInputBlocking(enabled)) {
    mi->UpdateSettings<bool>("blockInputs", enabled);
    log(INFO, "Input blocker " + std::string(enabled ? "enabled" : "disabled"));
  } else {
    blockInputsSwitch->setChecked(false);
    blockInputsSwitch->setEnabled(false);
    QMessageBox::warning(nullptr, "Missing add-on",
                         "Requires InputBlockerDLL.dll. Please visit the wiki "
                         "for more information.");
  }
}

void SettingsMenu::toggleAutostart(int state) {
  std::string path =
      std::getenv("APPDATA") +
      std::string("/Microsoft/Windows/Start Menu/Programs/Startup/ReVox.lnk");

  if (state == (int)Qt::CheckState::Unchecked) {
    std::filesystem::remove(path);
    mi->UpdateSettings<bool>("startWithWindows", false);
    log(INFO, "Autostart with Windows disabled");
  } else {
    wchar_t exec[MAX_PATH];
    GetModuleFileName(NULL, exec, MAX_PATH);
    std::wstring wpath = std::wstring(path.begin(), path.end());
    const wchar_t *wpath_t = wpath.c_str();

    HRESULT hres;
    IShellLinkW *psl;

    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_PPV_ARGS(&psl));
    if (SUCCEEDED(hres)) {
      IPersistFile *ppf;

      psl->SetPath(exec);

      hres = psl->QueryInterface(IID_PPV_ARGS(&ppf));
      if (SUCCEEDED(hres)) {
        ppf->Save(wpath_t, TRUE);
        ppf->Release();
      }

      psl->Release();
    }

    mi->UpdateSettings<bool>("startWithWindows", true);
    log(INFO, "Autostart with Windows enabled");
  }
}

void SettingsMenu::checkForUpdates() {
  CUpdaterDialog *updater = new CUpdaterDialog(
      (QWidget *)this->parent(),
      "https://api.github.com/repos/andre-tm-hui/ReVox/releases",
      VER_NO);
  updater->setStyleSheet("QTextEdit {border: 1px solid #009090;}");
  log(INFO, "Checking for updates");
}

void SettingsMenu::toggleAutocheck(int state) {
  bool enabled = state == 0 ? false : true;
  mi->UpdateSettings<bool>("autocheckUpdates", enabled);
  log(INFO,
      "Autocheck for updates " + std::string(enabled ? "enabled" : "disabled"));
}

void SettingsMenu::setNewBufferSize(QString sizeStr) {
  mi->UpdateSettings("framesPerBuffer", sizeStr.toInt());
  mi->Reset();
  log(INFO, "Buffer size set to " + sizeStr.toStdString());
}

void SettingsMenu::setupSwitch(Switch **s, QWidget *parent, int offset) {
  *s = new Switch(parent);
  (*s)->move(635, 12 + offset);
  (*s)->resize(61, 36);
  (*s)->setText("");
}
