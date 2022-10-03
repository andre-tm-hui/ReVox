#ifndef AUTOTUNETAB_H
#define AUTOTUNETAB_H

#include <QWidget>
#include <QSignalMapper>
#include <QTabWidget>
#include <QMenu>
#include <QCheckBox>
#include <nlohmann/json.hpp>
#include <audiomanager.h>

using namespace nlohmann;

namespace Ui {
class AutotuneTab;
}

class AutotuneTab : public QWidget
{
    Q_OBJECT

public:
    explicit AutotuneTab(AudioManager *am, QTabWidget *tabWidget, QWidget *parent = nullptr);
    ~AutotuneTab();

    void SetValues(json *settings, bool animate);

private slots:
    void toggle(int state);

    void toggleNote(int note);
    void setScale(int opt);

private:
    Ui::AutotuneTab *ui;

    json *values = nullptr;
    AudioManager *am;
    QTabWidget *tabWidget;

    QMenu *menu;
    QSignalMapper *mapper, *mapperAct;

    enum Note { C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B };

    std::map<Note, std::vector<bool>> scales {
        {C, {true, false, true, false, true, true, false, true, false, true, false, true}},
        {Db, {true, true, false, true, false, true, true, false, true, false, true, false}},
        {D, {false, true, true, false, true, false, true, true, false, true, false, true}},
        {Eb, {true, false, true, true, false, true, false, true, true, false, true, false}},
        {E, {false, true, false, true, true, false, true, false, true, true, false, true}},
        {F, {true, false, true, false, true, true, false, true, false, true, true, false}},
        {Gb, {false, true, false, true, false, true, true, false, true, false, true, true}},
        {G, {true, false, true, false, true, false, true, true, false, true, false, true}},
        {Ab, {true, true, false, true, false, true, false, true, true, false, true, false}},
        {A, {false, true, true, false, true, false, true, false, true, true, false, true}},
        {Bb, {true, false, true, true, false, true, false, true, false, true, true, false}},
        {B, {false, true, false, true, true, false, true, false, true, false, true, true,}}
    };

    std::vector<QString> menuLabels {
        "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"
    };

    std::vector<bool> notes {false, false, false, false, false, false, false, false, false, false, false, false};
    std::vector<QCheckBox*> noteCheckboxes;
};

#endif // AUTOTUNETAB_H
