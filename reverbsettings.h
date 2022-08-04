#ifndef REVERBSETTINGS_H
#define REVERBSETTINGS_H

#include <QDialog>
#include "revmodel.h"

namespace Ui {
class ReverbSettings;
}

class ReverbSettings : public QDialog
{
    Q_OBJECT

public:
    explicit ReverbSettings(revmodel *reverb, QWidget *parent = nullptr);
    ~ReverbSettings();

private slots:
    void on_mix_valueChanged(int value);

    void on_roomsize_valueChanged(int value);

    void on_width_valueChanged(int value);

    void on_damp_valueChanged(int value);

private:
    Ui::ReverbSettings *ui;
    revmodel *reverb;
};

#endif // REVERBSETTINGS_H
