#include "waveformviewer.h"

WaveformViewer::WaveformViewer(int x, int y, QWidget *parent)
    : QWidget{parent}
{
    this->resize(QSize(x, y));
    this->width = x;
    this->height = y;

    smallest = new float[this->width];
    largest = new float[this->width];
    rms = new float[this->width];

    memset(smallest, 0, sizeof(float) * this->width);
    memset(largest, 0, sizeof(float) * this->width);
    memset(rms, 0, sizeof(float) * this->width);
}

void WaveformViewer::SetAudioClip()
{
    if (!std::filesystem::exists(path))
    {
        memset(smallest, 0, sizeof(float) * this->width);
        memset(largest, 0, sizeof(float) * this->width);
        memset(rms, 0, sizeof(float) * this->width);
    }
    else
    {
        SF_INFO info;
        info.channels = 2;
        info.samplerate = 48000;
        SNDFILE *file = sf_open(path.c_str(), SFM_READ, &info);

        length = info.frames;
        float *in = new float[1024], *all = new float[info.frames * info.channels];
        double ratio = info.frames * info.channels / (double)width;
        sf_count_t c = 1024;
        int reads = 0;
        while (true)
        {
            c = sf_read_float(file, in, 1024);
            if (c < 1024) break;
            memcpy(all + reads * c, in, sizeof(float) * c);
            reads++;
        }
        sf_close(file);
        delete[] in;

        double pos = 0.0, lastPos = 0.0;
        for (int i = 0; i < width; i++)
        {
            pos = (i+1) * ratio;

            float min = all[(int)lastPos], max = all[(int)lastPos], sum = 0.0f;
            for (int j = (int)lastPos; j < pos; j++)
            {
                sum += all[j] * all[j];
                if (all[j] < min) { min = all[j]; }
                if (all[j] > max) { max = all[j]; }
            }

            smallest[i] = min;
            largest[i] = max;
            if (sum > 0)
            {
                rms[i] = sqrt(sum / (pos - lastPos));
            }
            else
            {
                rms[i] = 0;
            }

            lastPos = pos;
        }

        delete[] all;
    }

    this->repaint();
}

void WaveformViewer::SetPath(std::string path)
{
    this->path = path;
}

void WaveformViewer::Clear()
{
    memset(smallest, 0, sizeof(float) * this->width);
    memset(largest, 0, sizeof(float) * this->width);
    memset(rms, 0, sizeof(float) * this->width);
    this->path = "";
}

void WaveformViewer::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QPen rangeLines(QColor("#00C0C0"), 1, Qt::SolidLine);
    QPen rmsLines(QColor("#80C0C0"), 1, Qt::SolidLine);
    for (int x = 0; x < width; x++)
    {
        painter.setPen(rangeLines);
        painter.drawLine(x, (int)((height / 2.f) - (smallest[x] * (height / 2.f))), x, (int)((height / 2.f) - (largest[x] * (height / 2.f))));
        painter.setPen(rmsLines);
        painter.drawLine(x, (int)((height / 2.f) - (rms[x] * (height / 2.f))), x, (int)((height / 2.f) + (rms[x] * (height / 2.f))));
    }
}
