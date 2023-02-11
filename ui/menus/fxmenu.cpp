#include "fxmenu.h"
#include "ui_fxmenu.h"

FXMenu::FXMenu(std::shared_ptr<FXManager> fm, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FXMenu),
    fm(fm)
{
    ui->setupUi(this);

    mapper = new QSignalMapper();

    auto fxs = fm->GetFXs();
    int i = 0;
    for (auto const& [fx,iFx] : *fxs) {
        FXTab *tab = new FXTab(fm, fx, ui->fxTabWidget);
        tabs.push_back(tab);
        ui->fxTabWidget->addTab(tab, QIcon(":/rc/fxMenu/FXOff.png"), QString::fromStdString(fx));

        auto widgets = iFx->GetFXWidgets();
        for (auto const& [param, widget] : widgets) {
            FXParam *fxp = nullptr;
            if (widget.type == "FXDial")
                fxp = new FXDial(fm, fx, param, widget.unit, nullptr, widget.min, widget.max);
            else if (widget.type == "FXKeyPicker")
                fxp = new FXKeyPicker(fm, fx, param, nullptr);
            else if (widget.type == "FXSwitch")
                fxp = new FXSwitch(fm, fx, param, nullptr);

            if (fxp != nullptr) {
                fxp->setToolTip(QString::fromStdString(widget.toolTip));
                tab->getParamBox()->layout()->addWidget(fxp);
            }
        }

        mapper->setMapping(tab, i++);
        connect(tab, SIGNAL(toggled()), mapper, SLOT(map()));
    }

    while (ui->fxTabWidget->count() > fxs->size())
        ui->fxTabWidget->removeTab(0);

    connect(mapper, SIGNAL(mappedInt(int)), this, SLOT(toggleTab(int)));

    hkl = new HotkeyListWidget(ui->hotkeysContainer->size(), fm, "hotkeys", ui->hotkeysContainer);
    connect(hkl, SIGNAL(onSelectionChanged(int)), this, SLOT(onHotkeySelect(int)));

    ui->monitorSlider->setValue(fm->GetSetting("monitor").get<int>());
    ui->monitorSlider->setLabel(":/rc/fxMenu/micMonitorLabel.png");
    connect(ui->monitorSlider, SIGNAL(valueChanged(int)), this, SLOT(setMonitorVolume(int)));
}

FXMenu::~FXMenu()
{
    delete ui;
}

void FXMenu::toggleTab(int idx) {
    if (tabs[idx]->isToggled())
       ui->fxTabWidget->setTabIcon(idx, QIcon(":/rc/fxMenu/FXOn.png"));
    else
       ui->fxTabWidget->setTabIcon(idx, QIcon(":/rc/fxMenu/FXOff.png"));
}

void FXMenu::onHotkeySelect(int idx) {
    if (idx >= 0) {
        for (auto tab : tabs) {
            tab->setIdx(idx);
        }

        bool editable = fm->GetSetting("hotkeys/" + std::to_string(idx) + "/editable").get<bool>();
        fade(this, ui->overlay, !editable, 200);
    } else {
        hkl->deselect();
        for (auto tab : tabs) {
            tab->resetIdx();
        }
        ui->overlay->show();
    }

    if (idx > 0) emit itemSelected();
}

void FXMenu::setMonitorVolume(int val) {
    fm->SetMonitoringVol(val);
}
