#ifndef SETTINGSMENU_H
#define SETTINGSMENU_H

#include <QWidget>
#include <QCheckBox>
#include <QSignalMapper>
#include <audiomanager.h>
#include <hud.h>
#include <switch.h>
#include <Windows.h>
#include <shlobj.h>

namespace Ui {
class SettingsMenu;
}

class SettingsMenu : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsMenu(AudioManager *am, HUD *hud, QWidget *parent = nullptr);
    ~SettingsMenu();

    void SetDevices(std::map<std::string, device> devices, int currentDevice);

private slots:
    void deviceChanged(int value);
    void hudPositionChanged(int item);
    void toggleAutostart(int state);

private:
    void populateDevices();

    Ui::SettingsMenu *ui;
    AudioManager *am;
    HUD *hud;
    QSignalMapper *mapper;
    std::map<std::string, device> devices;
    int currentDevice = 0;

    Switch *_switch;

    std::vector<QString> positionLabels = {"Top Left", "Top", "Top Right", "Left", "Right", "Bottom Left", "Bottom", "Bottom Right", "Off"};
    std::vector<QCheckBox*> positionsCheckboxes = {};
};

#endif // SETTINGSMENU_H
