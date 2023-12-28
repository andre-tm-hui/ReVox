#ifndef MONITORSLIDER_H
#define MONITORSLIDER_H

#include <QWidget>

#include "util/loggableobject.h"

namespace Ui {
class MonitorSlider;
}

class MonitorSlider : public QWidget, public LoggableObject {
  Q_OBJECT

 public:
  explicit MonitorSlider(QWidget *parent = nullptr);
  ~MonitorSlider();
  void setLabel(QString url);
  void setValue(int val);

 signals:
  void valueChanged(int val);

 private slots:
  void onValueChanged(int val);

 private:
  Ui::MonitorSlider *ui;

  const int edgeOffset = 3;
};

#endif  // MONITORSLIDER_H
