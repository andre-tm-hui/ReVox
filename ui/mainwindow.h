#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAudioDevice>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMainWindow>
#include <QMediaDevices>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSignalMapper>
#include <QSystemTrayIcon>

#include "../backend/keyboardlistener.h"
#include "../interface/maininterface.h"
#include "menus/fxmenu.h"
#include "menus/settingsmenu.h"
#include "menus/soundboardmenu.h"
#include "onboarding.h"
#include "titlebar.h"
#include "ui_mainwindow.h"
#include "util/keypresseater.h"
#include "util/qttransitions.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public LoggableObject {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  static MainInterface *mi;
  static KeyboardListener *keyboardListener;

  void setVisible(bool visible) override;
  std::map<std::string, QCheckBox *> checkboxes = {};

 protected:
  bool event(QEvent *event) override;

 private slots:
  void iconActivated(QSystemTrayIcon::ActivationReason reason);

  void close();

  void devicesChanged();

  void openMenu(int menu);

  void fadeInMenu();

  void resetAudio();

  void moveWindow(QMouseEvent *event, int x, int y);

  void restartTutorial();

 private:
  Ui::MainWindow *ui;

  enum Menu { settings = -1, soundboard = 0, fx = 1, voice = 2 };

  QListWidget *keybinds;
  QAction *minimizeAction, *maximizeAction, *restoreAction, *resetAction,
      *quitAction;
  QSystemTrayIcon *trayIcon;
  QMenu *trayIconMenu;
  QSignalMapper mapper_add, mapper_remove, mapper_fx, mapper_menu;
  QMediaDevices *devices;
  QWidget *activeMenu, *nextMenu;

  KeyPressEater *keyPressEater;

  SoundboardMenu *soundboardMenu;
  FXMenu *fxMenu;
  SettingsMenu *settingsMenu;

  HUD *hud;
  TitleBar *titleBar;

  Onboarding *onboarding;

  int selectedSoundboardHotkey, fxHotkey = -1;

  bool setup = false, dragging = false;
};
#endif  // MAINWINDOW_H
