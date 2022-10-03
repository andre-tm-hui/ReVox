#ifndef REVERBTAB_H
#define REVERBTAB_H

#include <QWidget>
#include <QTabWidget>
#include <nlohmann/json.hpp>
#include <customdial.h>
#include <audiomanager.h>
#include <qttransitions.h>

using namespace nlohmann;

namespace Ui {
class ReverbTab;
}

class ReverbTab : public QWidget
{
    Q_OBJECT

public:
    explicit ReverbTab(AudioManager *am, QTabWidget *tabWidget, QWidget *parent = nullptr);
    ~ReverbTab();

    void SetValues(json *values, bool animate = false);

private slots:
    void toggle(int state);

    void releaseWetdry();
    void releaseRoomsize();
    void releaseWidth();
    void releaseDamp();

    void updateWetdry(int value);
    void updateRoomsize(int value);
    void updateWidth(int value);
    void updateDamp(int value);

private:
    Ui::ReverbTab *ui;
    AudioManager *am;
    QTabWidget *tabWidget;
    CustomDial *wetdry, *roomsize, *width, *damp;
    json *values = nullptr;
};

#endif // REVERBTAB_H
