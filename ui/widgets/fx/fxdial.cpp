#include "fxdial.h"

#include "ui_fxdial.h"

FXDial::FXDial(std::shared_ptr<FXManager> fm, std::string fx, std::string param,
               std::string unit, QWidget *parent, int min, int max)
    : FXParam(fm, fx, param, parent), ui(new Ui::FXDial) {
  ui->setupUi(this);

  ui->dial->setMinimum(min);
  ui->dial->setMaximum(max);

  connect(ui->dial, SIGNAL(sliderReleased()), this, SLOT(onValueChanged()));
  connect(ui->dial, SIGNAL(valueChanged(int)), this, SLOT(updateDisplay(int)));

  ui->title->setText(QString::fromStdString(param));
  ui->unit->setText(QString::fromStdString(unit));
  resetIdx();
}

FXDial::~FXDial() { delete ui; }

void FXDial::setValue(int val) {
  disconnect(ui->dial, SIGNAL(sliderReleased()), this, SLOT(onValueChanged()));
  updateDisplay(val);
  QPropertyAnimation *anim = slideToValue(ui->dial, val, 100);
  connect(anim, SIGNAL(finished()), this, SLOT(reEnableUpdates()));
  fm->ApplyFXSettings({{fx, {{param, val}}}});
}

void FXDial::onValueChanged() {
  if (idx == -1) return;
  int val = ui->dial->value();
  fm->UpdateSettings<int>(getPath(), val);
  fm->ApplyFXSettings({{fx, {{param, val}}}});
  emit valueChanged(val);
  log(INFO, "Value changed to " + std::to_string(val));
}

void FXDial::updateDisplay(int val) { ui->value->display(val); }

void FXDial::reEnableUpdates() {
  connect(ui->dial, SIGNAL(sliderReleased()), this, SLOT(onValueChanged()));
}

void FXDial::resizeEvent(QResizeEvent *event) {
  ui->dial->move((this->width() / 2) - (ui->dial->width() / 2), ui->dial->y());
  ui->title->resize(this->width(), ui->title->height());
  ui->value->move((this->width() / 2) - (ui->value->width() / 2),
                  ui->value->y());
  ui->unit->move(ui->value->x() + 32, ui->unit->y());

  FXParam::resizeEvent(event);
}
