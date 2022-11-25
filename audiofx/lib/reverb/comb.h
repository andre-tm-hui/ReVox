// Comb filter class declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef COMB_H
#define COMB_H

#include "denormals.h"
#include <math.h>
#include <stdio.h>

class comb
{
public:
                    comb();
            void	setbuffer(float *buf, int size);
    inline  float	process(float inp);
            void	mute();
            void	setdamp(float val);
            float	getdamp();
            void	setfeedback(float val);
            float	getfeedback();
private:
    float	feedback;
    float	filterstore;
    float	damp1;
    float	damp2;
    float	*buffer;
    int		bufsize;
    int		bufidx;
};


// Big to inline - but crucial for speed

inline float comb::process(float input)
{
    float output;

    output = buffer[bufidx];
    undenormalise(output);

    filterstore = (output*damp2) + (filterstore*damp1);
    undenormalise(filterstore);
    if (isnan(filterstore)){
        filterstore = 0.f;
    }

    buffer[bufidx] = input + (filterstore*feedback);
    if (isnan(buffer[bufidx])){
        buffer[bufidx] = 0.f;
    }
    if(++bufidx>=bufsize) bufidx = 0;

    return output;
}


#endif // COMB_H
