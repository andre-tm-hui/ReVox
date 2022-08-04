#ifndef PITCHSETTINGS_H
#define PITCHSETTINGS_H

#include <QDialog>
#include <pitchshift.h>

namespace Ui {
class PitchSettings;
}

class PitchSettings : public QDialog
{
    Q_OBJECT

public:
    explicit PitchSettings(PitchShift *pitchshift, QWidget *parent = nullptr);
    ~PitchSettings();

private slots:
    void onChange(int value);

private:
    Ui::PitchSettings *ui;
    PitchShift *pitchshift;
};

#endif // PITCHSETTINGS_H
