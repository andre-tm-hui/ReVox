#ifndef KEYBINDSETTINGS_H
#define KEYBINDSETTINGS_H

#include <QDialog>
#include <QDoubleValidator>
#include "audiomanager.h"

namespace Ui {
class KeybindSettings;
}

class KeybindSettings : public QDialog
{
    Q_OBJECT

public:
    explicit KeybindSettings(json *settings, AudioManager *audioManager, QWidget *parent = nullptr);
    ~KeybindSettings();

private slots:
    void on_buttonBox_accepted();

    void on_recordLoopback_stateChanged(int arg1);

private:
    Ui::KeybindSettings *ui;
    json *settings;
    AudioManager *audioManager;
};

#endif // KEYBINDSETTINGS_H
