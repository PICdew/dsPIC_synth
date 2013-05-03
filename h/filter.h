
#ifndef FILTER_H
#define	FILTER_H

void fltInit();

void setFltCut(int f, float cut);

void setFltRes(int f, float res);
void setFltMix(int f, float mix);

unsigned int filt(int f, fixed samp);

unsigned int ctlFilter(int data, float filterVal, float smoothedVal);

#endif	/* FILTER_H */

