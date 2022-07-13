#include "fifobuffer.h"
#include <stdio.h>

FIFOBuffer::FIFOBuffer(int size)
{
    this->size = size;
    this->buffer = new float[size];
    for (int i = 0; i < this->size; i++)
    {
        this->buffer[i] = 0;
    }
    fprintf(stdout, "fifo buffer made\n"); fflush(stdout);
}

void FIFOBuffer::SetSize(int size)
{
    if (size % 2 != 0) size++;
    fprintf(stdout, "fifo buffer size %d\n", size); fflush(stdout);
    float* newBuffer = new float[size];
    fprintf(stdout, "new buffer made\n"); fflush(stdout);
    for (int i = 0; i < size; i++)
    {
        if (size >= this->size)
        {
            newBuffer[i] = 0;
        }
        else
        {
            newBuffer[i] = this->buffer[i];
        }
    }
    fprintf(stdout, "buffer copied\n"); fflush(stdout);
    this->size = size;
    this->buffer = newBuffer;
    fprintf(stdout, "fifo buffer size set\n"); fflush(stdout);
}

void FIFOBuffer::Add(float dat)
{
    for (int i = 0; i < this->size - 1; i++)
    {
        this->buffer[i] = this->buffer[i + 1];
    }
    this->buffer[this->size - 1] = dat;

    if (std::abs(dat) > max)
    {
        max = std::abs(dat);
    }
}
