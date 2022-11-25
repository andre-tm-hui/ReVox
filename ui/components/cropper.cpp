#include "cropper.h"
#include <iostream>

Cropper::Cropper(QWidget *parent)
    : QWidget{parent}
{
    this->resize(this->parentWidget()->size());
    wv = new WaveformViewer(this);
    wv->resize(this->size());
    connect(wv, SIGNAL(transitionDone()), this, SLOT(waveformChanged()));

    l_slider = new ClickthroughSlider(this);
    l_slider->resize(this->parentWidget()->size());
    l_slider->raise();
    l_slider->setToolTip("Set the start time");
    connect(l_slider, SIGNAL(valueChanged(int)), this, SLOT(startTimeChanged(int)));

    r_slider = new ClickthroughSlider(this);
    r_slider->setDirection(false);
    r_slider->resize(this->parentWidget()->size());
    r_slider->setValue(this->parentWidget()->width() + 1);
    r_slider->raise();
    r_slider->setToolTip("Set the end time");
    connect(r_slider, SIGNAL(valueChanged(int)), this, SLOT(endTimeChanged(int)));

    label = new QLabel(this);
    label->raise();
    label->setText("--:--");
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("QLabel { background: rgba(0,0,0,192); font-size: 9px; color: #DDDDDD; font-family: \"Terminal\";}");
    label->resize(30, 12);
    label->move(this->width() - label->width(), this->height() - label->height());
    connect(wv, SIGNAL(transitionDone()), this, SLOT(setTimeLabel()));
}

void Cropper::resizeEvent(QResizeEvent *event) {
    wv->resize(this->size());
    l_slider->resize(this->size());
    r_slider->resize(this->size());
}

void Cropper::setAudioPath(std::string path, json values) {
    this->path = path;
    this->values = values;
    transitioning = true;
    QWidget::window()->setEnabled(false);
    animateValue(l_slider, 0);
    QPropertyAnimation *anim = animateValue(r_slider, r_slider->maximum());
    connect(anim, SIGNAL(finished()), this, SLOT(changeWaveform()));
}

QPropertyAnimation* Cropper::animateValue(ClickthroughSlider *slider, int targetValue, int duration) {
    QPropertyAnimation *anim = new QPropertyAnimation(slider, "sliderPosition");
    anim->setDuration(duration);
    anim->setStartValue(slider->value());
    anim->setEndValue(targetValue == -1 ? slider->maximum() : targetValue);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start();
    return anim;
}

void Cropper::changeWaveform() {
    wv->startTransition(path);
}

void Cropper::waveformChanged() {
    if (values == NULL || wv->getLength() == 0) { cleanup(); return; }
    animateValue(l_slider, l_slider->maximum() * (float)values["startAt"].get<int>() / wv->getLength());
    QPropertyAnimation *anim = animateValue(r_slider, values["endAt"].get<int>() == -1 ? r_slider->maximum()
                                                                                       : r_slider->maximum() * (float)values["endAt"].get<int>() / wv->getLength());
    connect(anim, SIGNAL(finished()), this, SLOT(cleanup()));
}

void Cropper::cleanup() {
    transitioning = false;
    QWidget::window()->setEnabled(true);
    if (wv->getLength() == 0)
        this->setEnabled(false);
    else {
        this->setEnabled(true);
        emit onClipChanged();
    }
}

int Cropper::sliderToFrames(int val) {
    return round(wv->getLength() * (float)val / l_slider->maximum());
}

void Cropper::setTimeLabel() {
    emit onTransitionComplete(wv->getLength() > 0);
    label->setText(wv->getTimeLabel());
}

void Cropper::startTimeChanged(int val) {
    val = __min(val, r_slider->value());
    l_slider->setValue(val);
    if (!transitioning) emit onStartTimeChanged(sliderToFrames(val));
}

void Cropper::endTimeChanged(int val) {
    val = __max(val, l_slider->value());
    r_slider->setValue(val);
    if (!transitioning) emit onEndTimeChanged(sliderToFrames(val));
}
