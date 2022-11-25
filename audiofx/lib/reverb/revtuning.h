// Reverb model tuning values
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef REVTUNING_H
#define REVTUNING_H

const int	numcombs		= 8;
const int	numallpasses	= 4;
const float	muted			= 0;
const float	fixedgain		= 0.015f;
const float scalewet		= 3;
const float scaledry		= 2;
const float scaledamp		= 0.4f;
const float scaleroom		= 0.28f;
const float offsetroom		= 0.7f;
const float initialroom		= 1.0f;
const float initialdamp		= 0.5f;
const float initialwet		= 1/scalewet;
const float initialdry		= 0;
const float initialwidth	= 1;
const float initialmode		= 0;
const float freezemode		= 0.5f;
const int	stereospread	= 25;

// These values assume 44.1KHz sample rate
// they will probably be OK for 48KHz sample rate
// but would need scaling for 96KHz (or other) sample rates.
// The values were obtained by listening tests.
const int combtuningL1		= 1216;
const int combtuningR1		= 1216+stereospread;
const int combtuningL2		= 1294;
const int combtuningR2		= 1294+stereospread;
const int combtuningL3		= 1392;
const int combtuningR3		= 1392+stereospread;
const int combtuningL4		= 1478;
const int combtuningR4		= 1478+stereospread;
const int combtuningL5		= 1550;
const int combtuningR5		= 1550+stereospread;
const int combtuningL6		= 1625;
const int combtuningR6		= 1625+stereospread;
const int combtuningL7		= 1697;
const int combtuningR7		= 1697+stereospread;
const int combtuningL8		= 1763;
const int combtuningR8		= 1763+stereospread;
const int allpasstuningL1	= 606;
const int allpasstuningR1	= 606+stereospread;
const int allpasstuningL2	= 481;
const int allpasstuningR2	= 481+stereospread;
const int allpasstuningL3	= 372;
const int allpasstuningR3	= 372+stereospread;
const int allpasstuningL4	= 245;
const int allpasstuningR4	= 245+stereospread;

#endif // REVTUNING_H
