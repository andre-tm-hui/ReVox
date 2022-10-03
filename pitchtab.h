#ifndef PITCHTAB_H
#define PITCHTAB_H

#include <QWidget>
#include <QTabWidget>
#include <nlohmann/json.hpp>
#include <customdial.h>
#include <audiomanager.h>
#include <qttransitions.h>

namespace Ui {
class PitchTab;
}

class PitchTab : public QWidget
{
    Q_OBJECT

public:
    explicit PitchTab(AudioManager *am, QTabWidget *tabWidget, QWidget *parent = nullptr);
    ~PitchTab();

    void SetValues(json *values, bool animate = false);

private slots:
    void toggle(int state);

    void releaseOffset();
    void updateOffset(int value);

private:
    Ui::PitchTab *ui;
    AudioManager *am;
    QTabWidget *tabWidget;
    CustomDial *dial;
    json *values = nullptr;
};

#endif // PITCHTAB_H
