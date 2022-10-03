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
    }

    connect(mapper, SIGNAL(mappedInt(int)), this, SLOT(hudPositionChanged(int)));
    connect(ui->devices, SIGNAL(currentIndexChanged(int)), this, SLOT(deviceChanged(int)));

    if (hud->GetPosition() >= 0)
        positionsCheckboxes[hud->GetPosition()]->setCheckState(Qt::CheckState::Checked);

    connect(ui->reset, SIGNAL(pressed()), this->parent(), SLOT(resetAudio()));

    _switch = new Switch(ui->autostartBox);
    _switch->move(635, 12);
    _switch->resize(61, 36);
    _switch->setText("");
    _switch->setChecked(am->settings["startWithWindows"]);
    connect(_switch, SIGNAL(stateChanged(int)), this, SLOT(toggleAutostart(int)));
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
    std::string path = std::getenv("APPDATA") + std::string("/Microsoft/Windows/Start Menu/Programs/Startup/vsns.lnk");

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
                hres = ppf->Save(startup, TRUE);
                ppf->Release();
            }

            psl->Release();
        }

        am->settings["startWithWindows"] = true;
    }
    am->SaveSettings();
}
