#include "waveformviewer.h"

WaveformViewer::WaveformViewer(QWidget *parent)
    : QWidget{parent}
{
    smallest = largest = rms = std::vector<float>(this->width(), 0);
    connect(this, SIGNAL(startRepaint()), this, SLOT(slotRepaint()), Qt::ConnectionType::QueuedConnection);
    connect(this, SIGNAL(transitionDone()), this, SLOT(cleanup()), Qt::ConnectionType::QueuedConnection);
}

void WaveformViewer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPen rangeLines(QColor("#00C0C0"), 1, Qt::SolidLine);
    QPen rmsLines(QColor("#80C0C0"), 1, Qt::SolidLine);
    for (int x = 0; x < this->width(); x++)
    {
        painter.setPen(rangeLines);
        painter.drawLine(x, (int)((this->height() / 2.f) - (smallest[x] * (this->height() / 2.f))), x, (int)((this->height() / 2.f) - (largest[x] * (this->height() / 2.f))));
        painter.setPen(rmsLines);
        painter.drawLine(x, (int)((this->height() / 2.f) - (rms[x] * (this->height() / 2.f))), x, (int)((this->height() / 2.f) + (rms[x] * (this->height() / 2.f))));
    }
}

void WaveformViewer::resizeEvent(QResizeEvent *event) {
    smallest = largest = rms = std::vector<float>(this->width(), 0);
    startTransition(path, 0, 0);

    QWidget::resizeEvent(event);
}

void WaveformViewer::transition(std::string path, int duration, int delay,
                                std::vector<float> *smallest,
                                std::vector<float> *largest,
                                std::vector<float> *rms,
                                WaveformViewer *wv,
                                int *length,
                                int *sampleRate) {
    std::vector<float> smallest_t, largest_t, rms_t;
    (*length) = 0;
    smallest_t = largest_t = rms_t = std::vector<float>(wv->width(), 0);
    if (path.size() > 0 && std::filesystem::exists(path)) {
        SF_INFO info;
        SNDFILE *file = sf_open(path.c_str(), SFM_READ, &info);

        float *all = new float[info.frames * info.channels];
        (*length) = info.frames;
        (*sampleRate) = info.samplerate;
        double ratio = info.frames * info.channels / (double)wv->width();
        sf_read_float(file, all, info.frames * info.channels);
        sf_close(file);

        double pos = 0.0, lastPos = 0.0;
        for (int i = 0; i < wv->width(); i++)
        {
            pos = (i+1) * ratio;

            float min = all[(int)lastPos], max = all[(int)lastPos], sum = 0.0f;
            for (int j = (int)lastPos; j < pos; j++)
            {
                sum += all[j] * all[j];
                if (all[j] < min) { min = all[j]; }
                if (all[j] > max) { max = all[j]; }
            }

            smallest_t[i] = min;
            largest_t[i] = max;
            if (sum > 0)
            {
                rms_t[i] = sqrt(sum / (pos - lastPos));
            }
            else
            {
                rms_t[i] = 0;
            }

            lastPos = pos;
        }

        delete[] all;
    }

    wv->setTimeLabel(true);
    if (duration > 0) {
        float timestep = 1;
        float interval = (float)wv->width() / duration;
        int iTimestep, iInterval;
        if (interval < 1) {
            timestep *= 1.f / interval;
            interval = 1.f;
        }
        iTimestep = round(timestep);
        iInterval = round(interval);
        int idx = - (iInterval * delay / iTimestep);

        for (int j = 0; idx < (int)smallest_t.size(); idx+=iInterval, j+=iInterval) {
            for (int k = 0; k < iInterval; k++) {
                if (idx + k >= 0 && idx + k < smallest->size()) {
                    (*smallest)[idx + k] = smallest_t[idx + k];
                    (*largest)[idx + k] = largest_t[idx + k];
                    (*rms)[idx + k] = rms_t[idx + k];
                }
                if (idx != j && j + k < smallest->size()) {
                    (*smallest)[j + k] = 0;
                    (*largest)[j + k] = 0;
                    (*rms)[j + k] = 0;
                }
            }

            emit wv->startRepaint();
            std::this_thread::sleep_for(std::chrono::milliseconds(iTimestep));
        }
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        (*smallest) = smallest_t;
        (*largest) = largest_t;
        (*rms) = rms_t;
    }
    wv->setTimeLabel();
    emit wv->transitionDone();
}

void WaveformViewer::startTransition(std::string path, int delay, int duration) {
    this->path = path;
    QWidget::window()->setEnabled(false);
    std::thread t(transition, path, delay, duration,
                  &smallest, &largest, &rms, this, &length, &sampleRate);
    t.detach();
}

void WaveformViewer::refresh(std::string path) {
    if (path == this->path) startTransition(path);
}

QString WaveformViewer::currentTime() {
    int seconds = round((float)length / sampleRate);
    int minutes = seconds / 60;
    seconds %= 60;
    QString out = QString::number(minutes) + ":" + QString::number(seconds);
    if (seconds < 10)
        out.insert(out.size()-1, '0');
    return out;
}

void WaveformViewer::setTimeLabel(bool loading) {
    if (loading || length == 0)
        timeLabel = "--:--";
    else
        timeLabel = currentTime();
}

void WaveformViewer::cleanup() {
    QWidget::window()->setEnabled(true);
    if (length == 0) this->setEnabled(false);
}
