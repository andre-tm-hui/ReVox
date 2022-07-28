#ifndef WINDOWFUNCTIONS_H
#define WINDOWFUNCTIONS_H

#include <cmath>
#include <vector>
#include <stdio.h>
#include <math.h>

namespace Window
{
    static void Hann(float* in, float* out, int n)
    {
        for (int i = 0; i < n; i++)
        {
            out[i] = in[i] * 0.5f * (1.f - cos(2.f * M_PI * (float)i / (float)n));
        }
        //fprintf(stdout, "\nhann\n"); fflush(stdout);
    }

    static std::vector<float> Hann(std::vector<float> in)
    {
        int n = in.size();
        for (int i = 0; i < n; i++)
        {
            in[i] = in[i] * 0.5 * (1 - cos(2 * M_PI * i / n));
        }
        return in;
    }
}

static float linterp(float a, float b, float d)
{
    return a + d * (b - a);
}

static float magnitude(float a, float b)
{
    return sqrt(a * a + b * b);
}

static float wrap(float a)
{
    return fmod(a + M_PI, 2.f * M_PI) - M_PI;
}

static int argmax(float *arr, int size)
{
    return std::distance(arr, std::max_element(arr, arr + size));
}

#endif // WINDOWFUNCTIONS_H
