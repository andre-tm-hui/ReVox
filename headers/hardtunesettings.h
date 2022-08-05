#ifndef HARDTUNESETTINGS_H
#define HARDTUNESETTINGS_H

#include <QDialog>
#include <QSignalMapper>
#include <pitchshift.h>
#include <keypresseater.h>

namespace Ui {
class HardtuneSettings;
}

class HardtuneSettings : public QDialog
{
    Q_OBJECT

public:
    explicit HardtuneSettings(PitchShift *autotune, QWidget *parent = nullptr);
    ~HardtuneSettings();

private slots:
    void toggleNote(int note);
    void setScale(int note);

private:
    void setCheckboxes();

    Ui::HardtuneSettings *ui;
    PitchShift *autotune;

    QSignalMapper mapper;

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

    std::vector<bool> currentNotes;
};

#endif // HARDTUNESETTINGS_H
