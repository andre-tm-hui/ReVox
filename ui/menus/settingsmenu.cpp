#include "settingsmenu.h"
#include "ui_settingsmenu.h"

SettingsMenu::SettingsMenu(std::shared_ptr<MainInterface> mi, HUD *hud, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsMenu),
    mi(mi),
    hud(hud)
{
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

    for (int i = 0; i < 8; i++)
    {
        mapper->setMapping(positionsCheckboxes[i], i);
        connect(positionsCheckboxes[i], SIGNAL(stateChanged(int)), mapper, SLOT(map()));
    }

    int hudPosition = mi->GetSetting("hudPosition").get<int>();
    if (hudPosition != -1)
    {
        positionsCheckboxes[hudPosition]->setCheckState(Qt::CheckState::Checked);
        ui->hudPositionLabel->setText(positionLabels[hudPosition]);
    }

    connect(mapper, SIGNAL(mappedInt(int)), this, SLOT(hudPositionChanged(int)));

    if (hud->GetPosition() >= 0)
        positionsCheckboxes[hud->GetPosition()]->setCheckState(Qt::CheckState::Checked);

    connect(ui->devices, SIGNAL(currentIndexChanged(int)), this, SLOT(deviceChanged()));
    connect(ui->reset, SIGNAL(pressed()), this->parent(), SLOT(resetAudio()));

    autostartSwitch = new Switch(ui->autostartBox);
    autostartSwitch->move(635, 12);
    autostartSwitch->resize(61, 36);
    autostartSwitch->setText("");
    autostartSwitch->setChecked(mi->GetSetting("startWithWindows").get<bool>());
    connect(autostartSwitch, SIGNAL(stateChanged(int)), this, SLOT(toggleAutostart(int)));

    connect(ui->update, SIGNAL(pressed()), this, SLOT(checkForUpdates()));

    autocheckSwitch = new Switch(ui->updateBox);
    autocheckSwitch->move(635, 47);
    autocheckSwitch->resize(61, 36);
    autocheckSwitch->setText("");
    autocheckSwitch->setChecked(mi->GetSetting("autocheckUpdates").get<bool>());
    connect(autocheckSwitch, SIGNAL(stateChanged(int)), this, SLOT(toggleAutocheck(int)));

    if (mi->GetSetting("autocheckUpdates").get<bool>())
    {
        CUpdaterDialog *updater = new CUpdaterDialog((QWidget*)this->parent(), "https://github.com/andre-tm-hui/ReVox", VER_NO, true);
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

void SettingsMenu::deviceChanged()
{
    for (auto const& [name, device] : devices)
    {
        if (QString::fromStdString(name) == ui->devices->currentText())
        {
            currentDevice = device.id;
            mi->SetCurrentOutputDevice(currentDevice);
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
        mi->UpdateSettings<int>("hudPosition", -1);
    }
    else
    {
        hudPositionText = positionLabels[item];
        hud->SetPosition(item);
        mi->UpdateSettings<int>("hudPosition", item);
    }

    ui->hudPositionLabel->setText(hudPositionText);
}

void SettingsMenu::toggleAutostart(int state)
{
    std::string path = std::getenv("APPDATA") + std::string("/Microsoft/Windows/Start Menu/Programs/Startup/ReVox.lnk");

    if (state == (int)Qt::CheckState::Unchecked)
    {
        std::filesystem::remove(path);
        mi->UpdateSettings<bool>("startWithWindows", false);
    }
    else
    {
        wchar_t exec[MAX_PATH];
        GetModuleFileName(NULL, exec, MAX_PATH);
        std::wstring wpath = std::wstring(path.begin(), path.end());
        const wchar_t *wpath_t = wpath.c_str();

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
                ppf->Save(wpath_t, TRUE);
                ppf->Release();
            }

            psl->Release();
        }

        mi->UpdateSettings<bool>("startWithWindows", true);
    }
}

void SettingsMenu::checkForUpdates()
{
    CUpdaterDialog *updater = new CUpdaterDialog((QWidget*)this->parent(), "https://github.com/andre-tm-hui/ReVox", VER_NO);
    updater->setStyleSheet("QTextEdit {border: 1px solid #009090;}");
}

void SettingsMenu::toggleAutocheck(int state)
{
    mi->UpdateSettings<bool>("autocheckUpdates", state == 0 ? false : true);
}
