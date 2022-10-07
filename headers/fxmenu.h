#ifndef FXMENU_H
#define FXMENU_H

#include <QWidget>
#include <QListWidget>
#include <nlohmann/json.hpp>
#include <audiomanager.h>
#include <keyboardlistener.h>
#include <hotkeyitem.h>
#include <reverbtab.h>
#include <autotunetab.h>
#include <pitchtab.h>
#include <hud.h>

namespace Ui {
class FXMenu;
}

class FXMenu : public QWidget
{
    Q_OBJECT

public:
    explicit FXMenu(json *hotkeys,
                    AudioManager *am,
                    KeyboardListener *kl,
                    HUD *hud,
                    QWidget *parent = nullptr);
    ~FXMenu();

    void OnHotkeyToggle(int keycode, bool clear = true);
    void DisableTabWidget();

private slots:
    void onHotkeySelect();

    void addBindSlot();
    void removeBind();

    void onSliderChanged(int value);

private:
    void addBind(int keybind = -1, QString label = "");

    Ui::FXMenu *ui;
    json *hotkeys;
    AudioManager *am;
    KeyboardListener *kl;
    HUD *hud;

    ReverbTab *reverbTab;
    AutotuneTab *autotuneTab;
    PitchTab *pitchTab;

    int currHotkey = -1;
};

#endif // FXMENU_H
