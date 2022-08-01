#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QSystemTrayIcon>
#include <hotkeyitem.h>
#include <thread>
#include "ui_mainwindow.h"
#include "audiomanager.h"
#include "keyboardlistener.h"
#include "windows.h"
#include "keybindsettings.h"
#include "vkcodenames.h"
#include "devicesettings.h"

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
    void on_removeBind_clicked();

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void openDeviceSetup();

    void addBind(int type, int keybind = -1, QString label = "");



    void toggleReverb(int state);

    void toggleAutotune(int state);

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

    QSignalMapper mapper;

    static QString GetKeyName(int keybind);

    device_data *d_data;
};
#endif // MAINWINDOW_H
