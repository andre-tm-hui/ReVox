#ifndef FXTAB_H
#define FXTAB_H

#include <QWidget>

#include "../../interface/fxmanager.h"
#include "../util/qttransitions.h"
#include "../widgets/fx/fxparam.h"

namespace Ui {
class FXTab;
}

class FXTab : public QWidget, public LoggableObject {
  Q_OBJECT

 public:
  explicit FXTab(std::shared_ptr<FXManager> fm, std::string fx,
                 QWidget *parent = nullptr);
  ~FXTab();

  void setIdx(int idx);
  void resetIdx() { setIdx(0), idx = -1; }
  QWidget *getParamBox();
  bool isToggled();

 signals:
  void toggled();

 private slots:
  void stateChanged();

 private:
  Ui::FXTab *ui;

  std::shared_ptr<FXManager> fm;
  std::string fx;
  int idx = -1;
};

#endif  // FXTAB_H
