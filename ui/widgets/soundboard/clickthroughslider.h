#ifndef CLICKTHROUGHSLIDER_H
#define CLICKTHROUGHSLIDER_H

#include <QSlider>

class ClickthroughSlider : public QSlider {
  Q_OBJECT

 public:
  ClickthroughSlider(QWidget *parent);
  void setDirection(bool leftToRight);
  bool getDirection() { return leftToRight; }

 protected:
  void resizeEvent(QResizeEvent *event);
  void sliderChange(SliderChange change);

 private:
  void resetMasks();

  bool leftToRight = true;
  int offset = 4;
};

#endif  // CLICKTHROUGHSLIDER_H
