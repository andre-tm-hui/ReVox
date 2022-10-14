#include "settingsmenu.h"
#include "ui_settingsmenu.h"

SettingsMenu::SettingsMenu(AudioManager *am, HUD *hud, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsMenu)
{
    ui->setupUi(this);

    this->am = am;
    this->hud = hud;

    positionsCheckboxes.push_back(ui->topLeft);
    positionsCheckboxes.push_back(ui->top);
    positionsCheckboxes.push_back(ui->topRight);
    positionsCheckboxes.push_back(ui->left);
    positionsCheckboxes.push_back(ui->right);
    positionsCheckboxes.push_back(ui->bottomLeft);
    positionsCheckboxes.push_back(ui->bottom);
    positionsCheckboxes.push_back(ui->bottomRight);

    mapper = new QSignalMapper();

    for (int i = 0; i < 8; i++)
    {
        mapper->setMapping(positionsCheckboxes[i], i);
        connect(positionsCheckboxes[i], SIGNAL(stateChanged(int)), mapper, SLOT(map()));
    }

    if (am->settings["hudPosition"] != -1)
    {
        positionsCheckboxes[am->settings["hudPosition"]]->setCheckState(Qt::CheckState::Checked);
        ui->hudPositionLabel->setText(positionLabels[am->settings["hudPosition"].get<int>()]);
    }

    connect(mapper, SIGNAL(mappedInt(int)), this, SLOT(hudPositionChanged(int)));
    connect(ui->devices, SIGNAL(currentIndexChanged(int)), this, SLOT(deviceChanged(int)));

    if (hud->GetPosition() >= 0)
        positionsCheckboxes[hud->GetPosition()]->setCheckState(Qt::CheckState::Checked);

    connect(ui->reset, SIGNAL(pressed()), this->parent(), SLOT(resetAudio()));

    autostartSwitch = new Switch(ui->autostartBox);
    autostartSwitch->move(635, 12);
    autostartSwitch->resize(61, 36);
    autostartSwitch->setText("");
    autostartSwitch->setChecked(am->settings["startWithWindows"]);
    connect(autostartSwitch, SIGNAL(stateChanged(int)), this, SLOT(toggleAutostart(int)));

    connect(ui->update, SIGNAL(pressed()), this, SLOT(checkForUpdates()));

    autocheckSwitch = new Switch(ui->updateBox);
    autocheckSwitch->move(635, 47);
    autocheckSwitch->resize(61, 36);
    autocheckSwitch->setText("");
    autocheckSwitch->setChecked(am->settings["autocheckUpdates"]);
    connect(autocheckSwitch, SIGNAL(stateChanged(int)), this, SLOT(toggleAutocheck(int)));
    if (am->settings["autocheckUpdates"])
    {
        CUpdaterDialog *updater = new CUpdaterDialog((QWidget*)this->parent(), "https://github.com/onglez/ReVox", VER_NO, true);
        updater->setStyleSheet("QTextEdit {border: 1px solid #009090;}");
    }
}

SettingsMenu::~SettingsMenu()
{
    delete ui;
}

void SettingsMenu::SetDevices(std::map<std::string, device> devices, int currentDevice)
{
    this->devices = devices;
    this->currentDevice = currentDevice;
    populateDevices();
}

void SettingsMenu::populateDevices()
{
    ui->devices->blockSignals(true);
    ui->devices->clear();
    for (auto const& [name, device] : devices)
    {
        ui->devices->addItem(QString::fromStdString(name));
        if (device.id == currentDevice)
        {
            ui->devices->setCurrentIndex(ui->devices->count()-1);
        }
    }
    ui->devices->blockSignals(false);
}

void SettingsMenu::deviceChanged(int value)
{
    for (auto const& [name, device] : devices)
    {
        if (QString::fromStdString(name) == ui->devices->currentText())
        {
            currentDevice = device.id;
            am->SetCurrentOutputDevice(currentDevice);
            break;
        }
    }
}

void SettingsMenu::hudPositionChanged(int item)
{
    for (int i = 0; i < 8; i++)
    {
        if (i != item)
        {
            positionsCheckboxes[i]->blockSignals(true);
            positionsCheckboxes[i]->setCheckState(Qt::CheckState::Unchecked);
            positionsCheckboxes[i]->blockSignals(false);
        }
    }

    QString hudPositionText = "";
    if (positionsCheckboxes[item]->checkState() == Qt::CheckState::Unchecked)
    {
        hudPositionText = "Off";
        hud->SetPosition(-1);
        am->settings["hudPosition"] = -1;
    }
    else
    {
        hudPositionText = positionLabels[item];
        hud->SetPosition(item);
        am->settings["hudPosition"] = item;
    }

    ui->hudPositionLabel->setText(hudPositionText);
    am->SaveSettings();
}

void SettingsMenu::toggleAutostart(int state)
{
    std::string path = std::getenv("APPDATA") + std::string("/Microsoft/Windows/Start Menu/Programs/Startup/ReVox.lnk");

    if (state == (int)Qt::CheckState::Unchecked)
    {
        std::filesystem::remove(path);
        am->settings["startWithWindows"] = false;
    }
    else
    {
        wchar_t exec[MAX_PATH];
        GetModuleFileName(NULL, exec, MAX_PATH);
        wchar_t startup[MAX_PATH];
        std::mbstowcs(startup, path.c_str(), path.length());

        HRESULT hres;
        IShellLinkW *psl;

        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
        if (SUCCEEDED(hres))
        {
            IPersistFile *ppf;

            psl->SetPath(exec);

            hres = psl->QueryInterface(IID_PPV_ARGS(&ppf));
            if (SUCCEEDED(hres))
            {
                ppf->Save(startup, TRUE);
                ppf->Release();
            }

            psl->Release();
        }

        am->settings["startWithWindows"] = true;
    }
    am->SaveSettings();
}

void SettingsMenu::checkForUpdates()
{
    CUpdaterDialog *updater = new CUpdaterDialog((QWidget*)this->parent(), "https://github.com/onglez/ReVox", VER_NO);
    updater->setStyleSheet("QTextEdit {border: 1px solid #009090;}");
}

void SettingsMenu::toggleAutocheck(int state)
{
    am->settings["autocheckUpdates"] = state == (int)Qt::CheckState::Unchecked ? false : true;
    am->SaveSettings();
}
