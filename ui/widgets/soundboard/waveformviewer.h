#ifndef WAVEFORMVIEWER_H
#define WAVEFORMVIEWER_H

#include <QWidget>
#include <QPainter>
#include <QPen>
#include <QTime>
#include <sndfile.h>
#include <filesystem>
#include <samplerate.h>
#include <cmath>
#include <thread>

class WaveformViewer : public QWidget
{
    Q_OBJECT
public:
    explicit WaveformViewer(QWidget *parent = nullptr);
    int getLength() { return length; }
    void setTimeLabel(bool loading = false);
    void startTransition(std::string path = "", int delay = 200, int duration = 200);
    void refresh(std::string path);
    QString getTimeLabel() { return timeLabel; }

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void startRepaint();
    void transitionDone();

private slots:
    void slotRepaint() { this->update(); }
    void cleanup();

private:
    static void transition(std::string path, int duration, int delay,
                           std::vector<float> *smallest,
                           std::vector<float> *largest,
                           std::vector<float> *rms,
                           WaveformViewer *wv,
                           int *length,
                           int *sampleRate);
    QString currentTime();
    QString timeLabel = "--:--";

    std::string path;

    std::vector<float> smallest, largest, rms;

    int length = 0, sampleRate = 48000;
};

#endif // WAVEFORMVIEWER_H
