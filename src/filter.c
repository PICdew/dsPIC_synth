#include "../h/fixed.h"
#include "../h/filter.h"


typedef struct{
fixed fCut;
fixed fRes;
fixed fMix;
fixed clipFPlus;
fixed clipFMinus;

float cut;
float mix;
float res;

fixed fHeight;
fixed fSpeed;
fixed fDelay;

fixed smpl;

}filter;

filter fltr[2];

void fltInit(int f){

fltr[f].clipFPlus=i2fp(8);
fltr[f].clipFMinus=-fltr[f].clipFPlus;

fltr[f].cut=0.5;
fltr[f].mix=0.0;
fltr[f].res=0.8;

fltr[f].fCut=fl2fp(fltr[0].cut*fltr[0].cut);

fltr[f].fMix=fl2fp(fltr[f].mix);

float tmpRes = (1-(fltr[f].res));
fltr[f].fRes=fl2fp(1-tmpRes*tmpRes*tmpRes);

}

void setFltCut(int f, float cut){
    fltr[f].fCut=fl2fp(cut*cut);
}

void setFltRes(int f, float res){
        res = 1.0-res;
        fltr[f].fRes=fl2fp(1-(res*res*res));
}

void setFltMix(int f, float mix){
        fltr[f].fMix=fl2fp(mix);
}


unsigned int filt(int f, fixed samp){
    fltr[f].smpl = samp;
    fltr[f].smpl >>=4; //4
    fltr[f].smpl -= 0x800; //0x800
    //sample >>= (2); //4

    fixed lpin = fp_mul(fltr[f].smpl,FP_ONE-fltr[f].fMix);  //low-pass amount
    //fixed hpin = -fp_mul(fltr[f].smpl,fltr[f].fMix);  //high-pass amount

    fixed difr = fp_sub(lpin,fltr[f].fHeight);  //difference between last sample and current low pass sample
    fltr[f].fSpeed = fp_mul(fltr[f].fSpeed,fltr[f].fRes);
    fltr[f].fSpeed = fp_add(fltr[f].fSpeed,fp_mul(difr,fltr[f].fCut));
    fltr[f].fHeight+=fltr[f].fSpeed;
   // fltr[f].fHeight+=(fltr[f].fDelay-hpin);


    fltr[f].smpl=fltr[f].fHeight;

    if(fltr[f].fHeight>fltr[f].clipFPlus){fltr[f].fHeight=fltr[f].clipFPlus;}
    if(fltr[f].fHeight<fltr[f].clipFMinus){fltr[f].fHeight=fltr[f].clipFMinus;}

   // fltr[f].fDelay=hpin;

    fltr[f].smpl<<=3;
    //fltr[f].smpl<<=2;
    fltr[f].smpl+=0x8000;
    return fltr[f].smpl;
}


//ctlFilter takes midi CC messages, and performs one pole low pass filtering on them.
unsigned int ctlFilter(int data, float filterVal, float smoothedVal){
  if (filterVal > 1){      // check to make sure param's are within range
    filterVal = .99;
  }
  else if (filterVal <= 0){
    filterVal = 0;
  }

  smoothedVal = (data * (1 - filterVal)) + (smoothedVal  *  filterVal);

  return (int)smoothedVal;
}



