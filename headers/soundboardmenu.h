#ifndef SOUNDBOARDMENU_H
#define SOUNDBOARDMENU_H

#include <QWidget>
#include <QFileDialog>
#include <QSignalMapper>
#include <nlohmann/json.hpp>
#include <audiomanager.h>
#include <keyboardlistener.h>
#include <waveformviewer.h>
#include <hotkeyitem.h>
#include <qttransitions.h>

using namespace nlohmann;

enum rebindType {
    rebind_recordOver
};

typedef struct {
    AudioManager *am;
    QWidget *mainWindow;
    QWidget *mainThread;
} rebindData;

namespace Ui {
class SoundboardMenu;
}

class SoundboardMenu : public QWidget
{
    Q_OBJECT

public:
    explicit SoundboardMenu(json *hotkeys,
                            AudioManager *am,
                            KeyboardListener *kl,
                            QWidget *parent = nullptr);
    ~SoundboardMenu();

    WaveformViewer* GetWaveform() { return wv; }
    void DisableSettings();

signals:
    void addBindPressed();
    void itemSelected();
    void newKeySet();

private slots:
    void onHotkeySelect();
    void fadeInHotkey();
    void addBind(int keybind = -1, QString label = "");
    void removeBind();

    void setRecordMic();
    void setRecordSys();
    void setOverlap();
    void loadClip();

    void leftMarkerMoved(int value);
    void rightMarkerMoved(int value);
    void playSample();
    void stopSample();
    void volumeChanged(int value);

    void onSliderChanged(int value);

    void rebind(int i);
    void cleanupRebind();

private:
    static void WaitForKeyboardInput(void *widget, AudioManager *am);
    rebindData data;
    rebindType rbType;
    QSignalMapper *mapper;

    Ui::SoundboardMenu *ui;
    json *hotkeys;
    AudioManager *am;
    KeyboardListener *kl;
    WaveformViewer *wv;

    int currHotkey;
    bool changingClips = false;
};

#endif // SOUNDBOARDMENU_H
