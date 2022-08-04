#ifndef KEYBINDSETTINGS_H
#define KEYBINDSETTINGS_H

#include <QDialog>
#include <QFileDialog>
#include <QDoubleValidator>
#include "audiomanager.h"

namespace Ui {
class KeybindSettings;
}

class KeybindSettings : public QDialog
{
    Q_OBJECT

public:
    explicit KeybindSettings(int keycode, json *settings, AudioManager *audioManager, QWidget *parent = nullptr);
    ~KeybindSettings();

private slots:
    void on_buttonBox_accepted();

    void on_recordLoopback_stateChanged(int arg1);

    void openFile();

private:
    Ui::KeybindSettings *ui;
    json *settings;
    AudioManager *audioManager;
    int keycode;
    QString fname = "";
};

#endif // KEYBINDSETTINGS_H
