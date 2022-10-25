#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QMenu>
#include <QListWidget>
#include <QHBoxLayout>
#include <QSystemTrayIcon>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <hotkeyitem.h>
#include <thread>
#include "ui_mainwindow.h"
#include "audiomanager.h"
#include "keyboardlistener.h"
#include "windows.h"
#include "vkcodenames.h"
#include "keypresseater.h"
#include <soundboardmenu.h>
#include <fxmenu.h>
#include <settingsmenu.h>
#include <hud.h>
#include <qttransitions.h>
#include <titlebar.h>
#include <errdialog.h>
#include <onboarding.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static AudioManager* audioManager;
    static KeyboardListener* keyboardListener;

    void setVisible(bool visible) override;
    std::map<std::string, QCheckBox*> checkboxes = {};

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

private:
    Ui::MainWindow *ui;

    static std::map<QString, int> keycodeMap;

    QListWidget *keybinds;
    bool setup = false;
    bool dragging = false;

    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *resetAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QSignalMapper mapper_add, mapper_remove, mapper_fx, mapper_menu;

    static QString GetKeyName(int keybind);

    KeyPressEater *keyPressEater;

    QMediaDevices *devices;

    enum Menu {
        settings = -1,
        soundboard = 0,
        fx = 1,
        voice = 2
    };

    QWidget *activeMenu, *nextMenu;
    SoundboardMenu *soundboardMenu;
    FXMenu *fxMenu;
    SettingsMenu *settingsMenu;
    HUD *hud;
    TitleBar *titleBar;

    int selectedSoundboardHotkey;
    WaveformViewer *wv;

    int fxHotkey = -1;
};
#endif // MAINWINDOW_H
