#ifndef FXMENU_H
#define FXMENU_H

#include <QWidget>
#include <QSignalMapper>
#include "../components/fxtab.h"
#include "../widgets/hotkeylistwidget.h"
#include "../widgets/fx/fxdial.h"
#include "../widgets/fx/fxkeypicker.h"
#include "../interface/fxmanager.h"

namespace Ui {
class FXMenu;
}

class FXMenu : public QWidget
{
    Q_OBJECT

public:
    explicit FXMenu(std::shared_ptr<FXManager> fm, QWidget *parent = nullptr);
    ~FXMenu();

    void reset() { onHotkeySelect(-1); };

signals:
    void itemSelected();

private slots:
    void toggleTab(int idx);
    void onHotkeySelect(int idx);
    void setMonitorVolume(int val);

private:
    Ui::FXMenu *ui;
    std::shared_ptr<FXManager> fm;

    HotkeyListWidget *hkl;
    std::vector<FXParam*> params;
    std::vector<FXTab*> tabs;
    QSignalMapper *mapper;
};

#endif // FXMENU_H
