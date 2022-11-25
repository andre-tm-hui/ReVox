#include "fxtab.h"
#include "ui_fxtab.h"

FXTab::FXTab(std::shared_ptr<FXManager> fm, std::string fx, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FXTab),
    fm(fm),
    fx(fx)
{
    ui->setupUi(this);

    connect(ui->toggleSwitch, SIGNAL(stateChanged(int)), this, SLOT(stateChanged()));
}

FXTab::~FXTab()
{
    delete ui;
}

void FXTab::setIdx(int idx) {
    this->idx = idx;
    if (idx == -1) return;
    ui->toggleSwitch->setCheckState(fm->GetSetting("hotkeys/" + std::to_string(idx) + "/" + fx + "/enabled").get<bool>()
                                    ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    for (int i = 0; i < ui->paramsBox->layout()->count(); i++) {
        FXParam *param {static_cast<FXParam*>(ui->paramsBox->layout()->itemAt(i)->widget())};
        param->setIdx(idx);
    }
}

void FXTab::stateChanged() {
    bool enabled = ui->toggleSwitch->checkState() == Qt::CheckState::Checked ? true : false;
    fm->UpdateSettings("hotkeys/" + std::to_string(idx) + "/" + fx + "/enabled", enabled);
    fm->ApplyFXSettings({{fx, {{"enabled", enabled}}}});
    fade(this, ui->overlay, !enabled, 200);

    if (enabled) {
        ui->onLabel->setStyleSheet("border: 0px; color: #A0A0A0");
        ui->offLabel->setStyleSheet("border: 0px;");
    } else {
        ui->onLabel->setStyleSheet("border: 0px;");
        ui->offLabel->setStyleSheet("border: 0px; color: #808080");
    }

    emit toggled();
}

QWidget* FXTab::getParamBox() {
    return ui->paramsBox;
}

bool FXTab::isToggled() {
    return ui->toggleSwitch->checkState() == Qt::CheckState::Checked ? true : false;
}
