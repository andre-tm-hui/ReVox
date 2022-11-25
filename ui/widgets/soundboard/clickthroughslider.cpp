#include "clickthroughslider.h"
#include <iostream>

ClickthroughSlider::ClickthroughSlider(QWidget *parent) : QSlider(parent)
{
    this->resize(this->parentWidget()->size());
    this->setOrientation(Qt::Orientation::Horizontal);
    this->setMaximum(this->width()+1);
    this->setPageStep(0);
    setDirection(true);
}

void ClickthroughSlider::setDirection(bool leftToRight) {
    this->leftToRight = leftToRight;
    if (leftToRight) {
        this->setMask(QRegion(0, 0, offset, this->parentWidget()->height()));
        this->setStyleSheet("QSlider {background: transparent; position: absolute; left: -4px; right: 4px;}"
                            "QSlider::groove:horizontal {background: rgba(0,0,0,0);}"
                            "QSlider::handle:horizontal {background: transparent; margin: 0 -4px;}"
                            "QSlider::handle:hover:horizontal {background: rgba(255,255,255,128); margin: 0 -4px;}"
                            "QSlider::sub-page:horizontal {background: rgba(0,0,0,180);}");
    } else {
        this->setValue(this->maximum());
        this->setStyleSheet("QSlider {background: transparent; position: absolute; left: -4px; right: 4px;}"
                            "QSlider::groove:horizontal {background: rgba(0,0,0,0);}"
                            "QSlider::handle:horizontal {background: transparent; margin: 0 -4px;}"
                            "QSlider::handle:hover:horizontal {background: rgba(255,255,255,128); margin: 0 -4px;}"
                            "QSlider::add-page:horizontal {background: rgba(0,0,0,180);}");
    }
}

void ClickthroughSlider::resizeEvent(QResizeEvent *event) {
    this->setMaximum(this->width()+1);
    resetMasks();
    QAbstractSlider::resizeEvent(event);
}

void ClickthroughSlider::sliderChange(SliderChange change) {
    if (change == SliderChange::SliderValueChange) {
        resetMasks();
    }

    QAbstractSlider::sliderChange(change);
}

void ClickthroughSlider::resetMasks() {
    if (this->leftToRight)
        this->setMask(QRegion(0, 0, __max(offset, this->value()), this->parentWidget()->height()));
    else
        this->setMask(QRegion(__min(this->width() - offset, this->value()), 0, __max(offset, this->width() - this->value()), this->parentWidget()->height()));
}
