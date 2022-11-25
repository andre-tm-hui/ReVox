#ifndef FXKEYPICKER_H
#define FXKEYPICKER_H

#include <QMenu>
#include <QSignalMapper>
#include <QCheckBox>
#include "fxparam.h"

enum Note { C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B };
const std::vector<int> scales { 2741, 1387, 2774, 1453, 2906, 1717, 3434, 2773, 1451, 2902, 1709, 3418 };
const std::vector<QString> labels { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };

namespace Ui {
class FXKeyPicker;
}

class FXKeyPicker : public FXParam
{
    Q_OBJECT

public:
    explicit FXKeyPicker(std::shared_ptr<FXManager> fm,
                         std::string fx,
                         std::string param,
                         QWidget *parent = nullptr);
    ~FXKeyPicker();

    void setValue(int val) override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onKeyToggle(int key);
    void onScaleSet(int scale);

private:
    Ui::FXKeyPicker *ui;
    QMenu *menu;
    QSignalMapper *mapper, *mapperMenu;

    std::vector<QCheckBox*> noteCheckboxes;
};

#endif // FXKEYPICKER_H
