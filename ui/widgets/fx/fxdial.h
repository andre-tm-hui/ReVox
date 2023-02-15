#ifndef FXDIAL_H
#define FXDIAL_H

#include "../../util/qttransitions.h"
#include "fxparam.h"

namespace Ui {
class FXDial;
}

class FXDial : public FXParam {
  Q_OBJECT

 public:
  explicit FXDial(std::shared_ptr<FXManager> fm, std::string fx,
                  std::string param, std::string unit,
                  QWidget *parent = nullptr, int min = 0, int max = 100);
  ~FXDial();

  void setValue(int val) override;

 protected:
  void resizeEvent(QResizeEvent *event) override;

 private slots:
  void onValueChanged();
  void updateDisplay(int val);
  void reEnableUpdates();

 private:
  Ui::FXDial *ui;
};

#endif  // FXDIAL_H
