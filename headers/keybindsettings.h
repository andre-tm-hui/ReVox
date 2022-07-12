#ifndef KEYBINDSETTINGS_H
#define KEYBINDSETTINGS_H

#include <QDialog>
#include "audiomanager.h"

namespace Ui {
class KeybindSettings;
}

class KeybindSettings : public QDialog
{
    Q_OBJECT

public:
    explicit KeybindSettings(keybind *settings, AudioManager *audioManager, QWidget *parent = nullptr);
    ~KeybindSettings();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::KeybindSettings *ui;
    keybind *settings;
    AudioManager *audioManager;
};

#endif // KEYBINDSETTINGS_H
