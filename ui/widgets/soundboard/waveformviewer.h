#ifndef WAVEFORMVIEWER_H
#define WAVEFORMVIEWER_H

#include <samplerate.h>
#include <sndfile.h>

#include <QPainter>
#include <QPen>
#include <QTime>
#include <QWidget>
#include <cmath>
#include <filesystem>
#include <thread>

#include "../../../util/loggableobject.h"

class WaveformViewer : public QWidget, public LoggableObject {
  Q_OBJECT
 public:
  explicit WaveformViewer(QWidget *parent = nullptr);
  int getLength() { return length; }
  void setTimeLabel(bool loading = false);
  void startTransition(std::string path = "", int delay = 200,
                       int duration = 200);
  void refresh(std::string path, WaveformViewer *);
  QString getTimeLabel() { return timeLabel; }

 protected:
  void paintEvent(QPaintEvent *) override;
  void resizeEvent(QResizeEvent *event) override;

 signals:
  void startRepaint();
  void transitionDone();
  void signalTransition();

 private slots:
  void slotRepaint() { this->update(); }
  void cleanup();
  void slotTransition() { startTransition(this->path); }

 private:
  static void transition(std::string path, int duration, int delay,
                         std::vector<float> *smallest,
                         std::vector<float> *largest, std::vector<float> *rms,
                         WaveformViewer *wv, int *length, int *sampleRate);
  QString currentTime();
  QString timeLabel = "--:--";

  std::string path;

  std::vector<float> smallest, largest, rms;

  int length = 0, sampleRate = 48000;
};

#endif  // WAVEFORMVIEWER_H
