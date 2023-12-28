#ifndef SETTINGSMENU_H
#define SETTINGSMENU_H

#include <Windows.h>
#include <cupdaterdialog.h>
#include <shlobj.h>

#include <QCheckBox>
#include <QSignalMapper>
#include <QWidget>

#include "../hud.h"
#include "interface/maininterface.h"
#include "../widgets/switch.h"

namespace Ui {
class SettingsMenu;
}

class SettingsMenu : public QWidget, public LoggableObject {
  Q_OBJECT

 public:
  explicit SettingsMenu(std::shared_ptr<MainInterface> mi, HUD *hud,
                        QWidget *parent = nullptr);
  ~SettingsMenu();

  void SetDevices(std::map<std::string, device> &devices, int currentDevice);

 private slots:
  void deviceChanged();
  void hudPositionChanged(int item);
  void toggleKeyboardDetector(int state);
  void toggleInputBlocker(int state);
  void toggleAutostart(int state);
  void toggleAutocheck(int state);
  void checkForUpdates();
  void setNewBufferSize(QString sizeStr);

 private:
  void populateDevices();
  void setupSwitch(Switch **s, QWidget *parent, int offset = 0);

  Ui::SettingsMenu *ui;
  std::shared_ptr<MainInterface> mi;
  HUD *hud;
  QSignalMapper *mapper;
  std::map<std::string, device> devices;
  int currentDevice = 0;

  Switch *keyboardDetectorSwitch, *blockInputsSwitch, *autostartSwitch,
      *autocheckSwitch;

  std::vector<QString> positionLabels = {
      "Top Left",    "Top",    "Top Right",    "Left", "Right",
      "Bottom Left", "Bottom", "Bottom Right", "Off"};
  std::vector<QCheckBox *> positionsCheckboxes = {};
};

#endif  // SETTINGSMENU_H
