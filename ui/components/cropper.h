#ifndef CROPPER_H
#define CROPPER_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include "json.hpp"
#include "../widgets/soundboard/clickthroughslider.h"
#include "../widgets/soundboard/waveformviewer.h"

using namespace nlohmann;

class Cropper : public QWidget
{
    Q_OBJECT
public:
    explicit Cropper(QWidget *parent = nullptr);

    void setAudioPath(std::string path, json values);
    WaveformViewer* getWaveformViewer() { return wv; }
    int getLength() { return wv->getLength(); }
    int getSliderMax() { return l_slider->maximum(); }

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void onClipChanged();
    void onTransitionComplete(bool valid);
    void onStartTimeChanged(int val);
    void onEndTimeChanged(int val);

private slots:
    void startTimeChanged(int val);
    void endTimeChanged(int val);
    void changeWaveform();
    void waveformChanged();
    void cleanup();
    void setTimeLabel();

private:
    int sliderToFrames(int val);
    QPropertyAnimation* animateValue(ClickthroughSlider *slider, int targetValue = -1, int duration = 100);

    QLabel *label;
    ClickthroughSlider *l_slider, *r_slider;
    WaveformViewer *wv;
    std::string path = "";
    bool transitioning = false;
    json values = NULL;

};

#endif // CROPPER_H
