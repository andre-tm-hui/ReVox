#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <thread>
#include "ui_mainwindow.h"
#include "audiomanager.h"
#include "keyboardlistener.h"
#include "windows.h"
#include "keybindsettings.h"

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
    void on_inputDevices_currentIndexChanged(int index);

    void on_outputDevices_currentIndexChanged(int index);

    void on_loopbackDevices_currentIndexChanged(int index);

    void on_virtualInputDevices_currentIndexChanged(int index);

    void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_addBind_clicked();

    void on_removeBind_clicked();

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void on_virtualOutputDevices_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;

    static std::map<QString, int> keycodeMap;
    QComboBox *inputDevices, *outputDevices, *loopbackDevices, *virtualInputDevices, *virtualOutputDevices;
    QTableWidget *keybinds;
    bool setup = false;

    static void WaitForKeyboardInput(QTableWidgetItem* item);

    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
};
#endif // MAINWINDOW_H
