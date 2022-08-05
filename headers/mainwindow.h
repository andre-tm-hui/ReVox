#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QHBoxLayout>
#include <QSystemTrayIcon>
#include <reverbsettings.h>
#include <hardtunesettings.h>
#include <pitchsettings.h>
#include <hotkeyitem.h>
#include <thread>
#include "ui_mainwindow.h"
#include "audiomanager.h"
#include "keyboardlistener.h"
#include "windows.h"
#include "keybindsettings.h"
#include "vkcodenames.h"
#include "devicesettings.h"
#include "keypresseater.h"

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

protected:
    bool event(QEvent *event) override;

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void openDeviceSetup();

    void addBind(int type, int keybind = -1, QString label = "");

    void removeBind(int type);

    void toggleReverb(int state);

    void toggleAutotune(int state);

    void togglePitchshift(int state);

    void openFXSettings(int type);

    void on_maxPlaybackLength_valueChanged(int arg1);

    void on_maxOverlappingSounds_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;

    static std::map<QString, int> keycodeMap;
    std::map<std::string, QCheckBox*> checkboxes = {};
    QListWidget *keybinds;
    bool setup = false;

    static void WaitForKeyboardInput(QListWidgetItem* item);

    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QSignalMapper mapper_add, mapper_remove, mapper_fx;

    static QString GetKeyName(int keybind);

    device_data *d_data;

    KeyPressEater *keyPressEater;
};
#endif // MAINWINDOW_H
