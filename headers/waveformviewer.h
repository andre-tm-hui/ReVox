#ifndef WAVEFORMVIEWER_H
#define WAVEFORMVIEWER_H

#include <QWidget>
#include <QPainter>
#include <QPen>
#include <sndfile.h>
#include <filesystem>
#include <samplerate.h>
#include <cmath>

class WaveformViewer : public QWidget
{
    Q_OBJECT
public:
    explicit WaveformViewer(int x, int y, QWidget *parent = nullptr);
    void SetPath(std::string path);
    void SetAudioClip();
    void Clear();
    virtual void paintEvent(QPaintEvent*);

    int GetLength() { return length; };

signals:

private:
    std::string path;
    float *smallest, *largest, *rms;

    int width, height, length;
};

#endif // WAVEFORMVIEWER_H
