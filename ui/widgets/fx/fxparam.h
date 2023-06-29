#ifndef FXPARAM_H
#define FXPARAM_H

#include <QWidget>

#include "../../../interface/fxmanager.h"

class FXParam : public QWidget, public LoggableObject {
  Q_OBJECT
 public:
  explicit FXParam(std::shared_ptr<FXManager> fm, std::string fx,
                   std::string param, QWidget *parent = nullptr);
  virtual void setValue(int val) {}
  void setIdx(int idx);
  void resetIdx();

 signals:
  void valueChanged(int val);

 protected:
  std::string getPath();

  std::shared_ptr<FXManager> fm;
  std::string fx, param;
  int idx = -1;
};

#endif  // FXPARAM_H
