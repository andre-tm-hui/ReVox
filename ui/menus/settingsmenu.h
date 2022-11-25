#ifndef SETTINGSMENU_H
#define SETTINGSMENU_H

#include <Windows.h>
#include <shlobj.h>
#include <cupdaterdialog.h>
#include <QWidget>
#include <QCheckBox>
#include <QSignalMapper>
#include "../interface/maininterface.h"
#include "../hud.h"
#include "../widgets/switch.h"


namespace Ui {
class SettingsMenu;
}

class SettingsMenu : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsMenu(std::shared_ptr<MainInterface> mi, HUD *hud, QWidget *parent = nullptr);
    ~SettingsMenu();

    void SetDevices(std::map<std::string, device> devices, int currentDevice);

private slots:
    void deviceChanged();
    void hudPositionChanged(int item);
    void toggleAutostart(int state);
    void toggleAutocheck(int state);
    void checkForUpdates();

private:
    void populateDevices();

    Ui::SettingsMenu *ui;
    std::shared_ptr<MainInterface> mi;
    HUD *hud;
    QSignalMapper *mapper;
    std::map<std::string, device> devices;
    int currentDevice = 0;

    Switch *autostartSwitch, *autocheckSwitch;

    std::vector<QString> positionLabels = {"Top Left", "Top", "Top Right", "Left", "Right", "Bottom Left", "Bottom", "Bottom Right", "Off"};
    std::vector<QCheckBox*> positionsCheckboxes = {};
};

#endif // SETTINGSMENU_H
