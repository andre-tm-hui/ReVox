#ifndef FIFOBUFFER_H
#define FIFOBUFFER_H

#include <cmath>

class FIFOBuffer
{
public:
    FIFOBuffer(int size = 1);
    void SetSize(int size);
    void Add(float dat);
    int size;
    float *buffer;
    float max = 0.000000000000000001;
};

#endif // FIFOBUFFER_H
