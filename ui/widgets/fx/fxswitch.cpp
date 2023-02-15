#include "fxswitch.h"

#include "ui_fxswitch.h"

FXSwitch::FXSwitch(std::shared_ptr<FXManager> fm, std::string fx,
                   std::string param, QWidget *parent)
    : FXParam(fm, fx, param, parent), ui(new Ui::FXSwitch) {
  ui->setupUi(this);

  connect(ui->toggle, SIGNAL(stateChanged(int)), this, SLOT(onToggled(int)));

  ui->title->setText(QString::fromStdString(param));
  ui->indicator->setEnabled(false);

  resetIdx();
}

FXSwitch::~FXSwitch() { delete ui; }

void FXSwitch::setValue(int val) {
  ui->toggle->setCheckState(val == 0 ? Qt::CheckState::Unchecked
                                     : Qt::CheckState::Checked);
}

void FXSwitch::resizeEvent(QResizeEvent *event) {
  ui->toggle->move((this->width() / 2) - (ui->toggle->width() / 2),
                   ui->toggle->y());
  ui->indicator->move((this->width() / 2) - (ui->indicator->width() / 2),
                      ui->indicator->y());
  ui->title->resize(this->width(), ui->title->height());
  FXParam::resizeEvent(event);
}

void FXSwitch::onToggled(int val) {
  if (idx == -1) return;
  ui->indicator->setCheckState((Qt::CheckState)val);
  val = val == 0 ? 0 : 1;
  fm->UpdateSettings<int>(getPath(), val);
  fm->ApplyFXSettings({{fx, {{param, val}}}});
  log(INFO, std::string(val == 0 ? "Switched off" : "Switched on"));
}
