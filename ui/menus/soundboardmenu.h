#ifndef SOUNDBOARDMENU_H
#define SOUNDBOARDMENU_H

#include <QWidget>
#include <QFileDialog>
#include "../interface/soundboardmanager.h"
#include "../widgets/bindablebutton.h"
#include "../widgets/hotkeylistwidget.h"
#include "../components/cropper.h"

namespace Ui {
class SoundboardMenu;
}

class SoundboardMenu : public QWidget
{
    Q_OBJECT

public:
    explicit SoundboardMenu(std::shared_ptr<SoundboardManager> sm, QWidget *parent = nullptr);
    ~SoundboardMenu();

    void reset() { setCurrentHotkey(-1); }

signals:
    void itemAdded();
    void itemSelected();
    void clipRecorded();

private slots:
    void setCurrentHotkey(int val);
    void setStartTime(int val);
    void setEndTime(int val);
    void setRecMic(bool val);
    void setRecSystem(bool val);
    void setOverlap(bool val);
    void setClipVolume(int val);
    void setMonitorVolume(int val);
    void setPadding(double val);
    void play();
    void stop();
    void loadFile();
    void delFile();

    void resetAll();

    void toggleButtons(bool enabled);

private:
    void settingsEnabled(bool enabled);

    Ui::SoundboardMenu *ui;
    std::shared_ptr<SoundboardManager> sm;

    HotkeyListWidget *hkl;
    Cropper *crp;
    BindableButton *modKey;

    int currHotkey = -1;
};

#endif // SOUNDBOARDMENU_H
