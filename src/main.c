#include "p33fj128gp804.h"
#include "dsp.h"
#include "../h/adcdacDrv.h"
#include "../h/midi.h"
#include "../h/fixed.h"
#include "../h/filter.h"
#include "../h/wavetables.h"
//#include <stdlib.h>

//#define RAND_MAX 100;


_FOSCSEL(FNOSC_FRC);		// FRC Oscillator
_FOSC(FCKSM_CSECMD & OSCIOFNC_ON  & POSCMD_NONE);
_FWDT(FWDTEN_OFF);    // Watchdog Timer Enabled/disabled by user software

static unsigned int square1[]={
127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,176,178,181,184,187,190,192,195,198,200,203,205,208,210,212,215,217,219,221,223,225,227,229,231,233,234,236,238,239,240,
  242,243,244,245,247,248,249,249,250,251,252,252,253,253,253,254,254,254,254,254,254,254,253,253,253,252,252,251,250,249,249,248,247,245,244,243,242,240,239,238,236,234,233,231,229,227,225,223,
  221,219,217,215,212,210,208,205,203,200,198,195,192,190,187,184,181,178,176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,127,124,121,118,115,111,108,105,102,99,96,93,90,87,84,81,78,
  76,73,70,67,64,62,59,56,54,51,49,46,44,42,39,37,35,33,31,29,27,25,23,21,20,18,16,15,14,12,11,10,9,7,6,5,5,4,3,2,2,1,1,1,0,0,0,0,0,0,0,1,1,1,2,2,3,4,5,5,6,7,9,10,11,12,14,15,16,18,20,21,23,25,27,29,31,
  33,35,37,39,42,44,46,49,51,54,56,59,62,64,67,70,73,76,78,81,84,87,90,93,96,99,102,105,108,111,115,118,121,124
};

typedef struct{
    unsigned char waveform;
    unsigned int freq;
    unsigned int phaseInc;
    unsigned int phase;
   // unsigned short amp;
    unsigned short duty;
    fixed value;
} oscillator;
unsigned char data;
unsigned char note;
char noteStack[]={-1,-1,-1,-1};
int lfof;

typedef struct{
	unsigned char status;
	unsigned char note;
	unsigned char velocity;
} midi_t;
typedef struct{
	 long time;
	 long gain;
	 long attack;
	//unsigned char decay;
	//unsigned char sustain;
	 long release;
	 long release_gain;
} adsr_t;
typedef struct{
    char buff[128];
    int write_index;
    int read_index;
    int unread;
}buffer;

buffer buff_str;
buffer* midiBuff = &buff_str;

typedef struct{
    fixed buff[512];
  unsigned  int write_index;
  unsigned  int read_index;
    int unread;
}sample_buffer;

sample_buffer sbuff_str;
sample_buffer* sampBuff = &sbuff_str;

//#define BUFF_MASK 31;

int tmpAttack;
int tmpRelease;

adsr_t adsr_str;
adsr_t* adsr = &adsr_str;

#define RX_SIZE 8
#define RX_MASK RX_SIZE-1
volatile unsigned char rx[3];
volatile unsigned char rx_pos;

volatile int currentNote;
volatile int cc[127];
volatile unsigned int midiClk;
volatile char midi_chan = 0x00;
volatile midi_t prev_midi;

 float mtof[128];
 float mtolfo[128];
 int x;
 int i;
 int j;
 unsigned char arpOn;
 unsigned char lfoOn;
 unsigned short filtarOn;
 int tick;
unsigned char step;
char arp[]={0,3,7};

int oscMix;
 oscillator osc[3];
 fixed output;
 short bend;
 int lfoffset;
short fMix;
short fCut;
short fRes;
short fAttack;
short fDecay;
short tmpfAttack;
short tmpfRelease;

 unsigned char fmAmt;

 char pClickState[]={0,0,0,0};
 unsigned char page;
 

int main (void)
{
        initPic();
        initTimer1();
        initTimer2();
        pinConfig();
	initDac();
        initMidi();
        fltInit(0);

        tick=2;
        step=0;

        fMix=0.0;
        fCut=255.0;
        fRes=0.0;
        fmAmt=0;

        tmpAttack  = adsr->attack = 64;
	//adsr->decay   = cc[9] = 64;
	//adsr->sustain = cc[10] = 127;
	tmpRelease  = adsr->release =  64;
	adsr->time = 0xFFFF;
 

        page=0;
        setColour(page);
        arpOn=0;
        lfoOn=0;
        filtarOn=0;
        note=40;
        bend=7;
        oscMix=127;
        osc[0].waveform=0;
        osc[1].waveform=200;
        osc[2].waveform=0;



         for(x=0; x<127; x++){
        mtof[x]=(440.0/32.0)*(pow(2,((x-9.0)/12.0)));
        mtolfo[x]=(440.0/32.0)*(pow(2,((x-9.0)/12.0)))/4;
           }
    

    while (1)      // Loop
    {
        //if(sampBuff->unread<512){calcSample();}
        if(midiBuff->unread>0){parseMidi();}

        if(PORTAbits.RA7!=pClickState[3]){ if(PORTAbits.RA7==0){page++; page%=3; setColour(page);} pClickState[3]=PORTAbits.RA7;}
        if(PORTAbits.RA2!=pClickState[1]){ if(PORTAbits.RA2==0&&page==1){if(arpOn==0){arpOn=1;}else{arpOn=0;}} pClickState[1]=PORTAbits.RA2;}
        if(PORTAbits.RA3!=pClickState[0]){ if(PORTAbits.RA3==0&&page==1){if(lfoOn==0){lfoOn=1;}else{lfoOn=0;}} pClickState[0]=PORTAbits.RA3;}
        if(PORTAbits.RA4!=pClickState[2]){ if(PORTAbits.RA4==0&&page==1){if(filtarOn==0){filtarOn=1;}else{filtarOn=0;}} pClickState[2]=PORTAbits.RA4;}
        switch(page){
            case 0:
                    bend -= read_enc(1);
                    if((bend+note)>127){bend--;}
                    if((bend+note)<0){bend++;}
                    oscMix += read_enc(2);
                    if(oscMix>255){oscMix=255;} if(oscMix<0){oscMix=0;}

                    osc[0].waveform+=read_enc(4);
                    osc[1].waveform+=read_enc(3);
                    break;
            case 1:
                    //fMix+=read_enc(4);
                    fCut-=read_enc(4);
                    if(fCut>255){fCut=255;}if(fCut<0){fCut=0;}
                    fRes-=read_enc(3);
                    if(fRes>225){fRes=225;}if(fRes<0){fRes=0;}
                    tick-=read_enc(2);
                    if(tick>255){tick=255;}if(tick<1){tick=1;}
                    lfoffset-=read_enc(1);
                    if(lfoffset>10){lfoffset=10;}if(lfoffset<-10){lfoffset=-10;}
                    //fmAmt+=read_enc(1);
                    break;
            case 2:
                tmpAttack+=read_enc(4); if(tmpAttack<=5){tmpAttack=5;}if(tmpAttack>255){tmpAttack=255;}
                tmpRelease+=read_enc(3); if(tmpRelease<=5){tmpRelease=5;}if(tmpRelease>255){tmpRelease=255;}
                break;
            default: break;
        }

        
        setFltMix(0,(fMix/255.0));
        
      if(filtarOn==0){setFltCut(0,(fCut/255.0));} else{setFltCut(0,(adsr->gain/127.0));}
        setFltRes(0,(fRes/255.0));
        
     //   LATAbits.LATA8=PORTAbits.RA3;
     //   LATAbits.LATA10=!PORTAbits.RA3;

       // osc[0].freq=mtof[note+arp[step]]+(osc[2].value); //Set oscs to freq
      //  osc[1].freq=mtof[note+bend+arp[step]]+(osc[2].value);
        if(arpOn==1){
        osc[0].freq=mtof[note+arp[step]]; //Set oscs to freq
        osc[1].freq=mtof[note+bend+arp[step]];
        osc[2].freq=mtolfo[note+arp[step]];
        }else{
        osc[0].freq=mtof[note]; //Set oscs to freq
        osc[1].freq=mtof[note+bend];
        osc[2].freq=mtolfo[note];
        }

        if(lfoOn==1){
        osc[1].freq+=(fp_mul(osc[2].value,(mtolfo[note]+lfoffset)));
        osc[0].freq+=(fp_mul(osc[2].value,(mtolfo[note]+lfoffset)));
        }

      //  osc[0].freq=mtof[note]; //Set oscs to freq
      //  osc[1].freq=mtof[note+bend];

        osc[0].phaseInc=phaseinc(osc[0].freq);
        osc[1].phaseInc=phaseinc(osc[1].freq);
        osc[2].phaseInc=phaseinc(osc[2].freq);
        calcAdsr(adsr);
        
    }

}

void __attribute__((interrupt, no_auto_psv)) _DAC1RInterrupt(void)
{
     IFS4bits.DAC1RIF = 0;// Clear Right Channel Interrupt Flag
     calcSample();

     // if(sampBuff->unread>0){
    // DAC1RDAT=sampBuffRead(sampBuff);
     //}
     
}

void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void){       
	IFS0bits.U1RXIF = 0; // clear RX interrupt flag
        buffWrite(midiBuff,U1RXREG);
       // parseMidi();

}
void parseMidi(){
        data=buffRead(midiBuff);
        if (data&0x80) rx_pos = 0;  //If data is MIDI status, set it as first byte.
        rx[rx_pos] = data;
        rx_pos = (rx_pos+1)&(RX_MASK);
        if(rx[0]==0xF8){
            midiClk++; midiClk%=(tick>>3)+1; if(midiClk==0){step++; step%=3;}
        }
        if(rx[0]==0xFA){midiClk=0;}
        if(rx_pos==3){
        if(rx[0]==(0x90)){
            if(rx[2]>0){note=rx[1]; noteon();}} //note on
        if(rx[0]==(0x90)){if(rx[2]==0){note=rx[1]; noteoff();}}//note off
        if(rx[0]==(0x80)){note=rx[1]; noteoff();}
        if(rx[0]==(0xB0)){
            switch(rx[1]){
                case 1: lfof=rx[2]; break;
                //case 2: setFltCut(0,(rx[2]/127.0)); break;
                //case 3: setFltRes(0,(rx[2]/127.0)); break;
            }
        }//cc

        }


}
void noteon(){
    osc[0].phase=0; //Set osc phase to 0 to prevent clipping (maybe?)
    osc[1].phase=0;

    //if(adsr->time==0xFFFF){adsr->time=0;}
    adsr->attack=tmpAttack;  //Start adsrs

    //   adsr->decay=cc[9]*2;
    //   adsr->sustain=cc[10];
    adsr->time=0;


}
void noteoff(){

            adsr->release=tmpRelease;
            adsr->time = (adsr->attack+1);

}
void calcAdsr(adsr_t* adsr){

	long temp = adsr->attack;
	if (adsr->time < temp){		//Attack
		adsr->release_gain = adsr->gain = ((adsr->time*127)/adsr->attack);
		adsr->time++;
		return;
	}

	if (adsr->time == temp){	//Sustain
		//adsr->release_gain = adsr->sustain;
		//adsr->gain = adsr->sustain;
		return;
	}
	temp += adsr->release+1;
	if (adsr->time < temp){		//Release
		adsr->gain = (adsr->release_gain*(temp-adsr->time))/adsr->release;
		adsr->time++;
		return;
	}
	else{
		adsr->gain = 0;
		adsr->time = 0xFFFF;
		return;
	}
}
void buffWrite(buffer* buffer, char value){
    buffer->buff[(buffer->write_index++)&127]=value;
    buffer->unread++;
}
void sampBuffWrite(sample_buffer* buffer, fixed value){
    buffer->buff[(buffer->write_index++)&511]=value;
    buffer->unread++;
}
int buffRead(buffer* buffer){
    buffer->unread--;
    int result=buffer->buff[(buffer->read_index)&127];
    buffer->read_index++;
    return result;
    
    
}
int sampBuffRead(sample_buffer* buffer){
    buffer->unread--;
    fixed result=buffer->buff[(buffer->read_index)&511];
    buffer->read_index++;
    return result;
}
void calcSample(){

     output=osc[0].value+osc[1].value;
     if(filtarOn==0){output=fp_mul(output,(adsr->gain));}else{output=fp_mul(output,127);}
     output=filt(0,output);
    // sampBuffWrite(sampBuff,output);
     DAC1RDAT=output;

     for(i=0; i<2; i++){
       //  osc[i].value=osc[i].phase;

/*square-table*/ if(osc[i].waveform<16){osc[i].value=square1[osc[i].phase>>8]*255;}
/*square-basic*/ if(osc[i].waveform>=16&&osc[i].waveform<32){if(osc[i].phase>32768){osc[i].value=0;}else {osc[i].value=65535;}}
/*saw----basic*/ if(osc[i].waveform>=32&&osc[i].waveform<64){osc[i].value=osc[i].phase;}
/*triangle----*/ if(osc[i].waveform>=64&&osc[i].waveform<128){if(osc[i].phase>=32768){osc[i].value=(65535-osc[i].phase)*2;}else{osc[i].value=osc[i].phase*2;}}
/*square---PWM*/ if(osc[i].waveform>=128){osc[i].phase>((osc[i].waveform-128)*512)?(osc[i].value=65500):(osc[i].value=0);}
     osc[i].phase+=osc[i].phaseInc;
     }
    osc[2].value=square1[osc[2].phase>>8];
    osc[2].phase+=osc[2].phaseInc;
    osc[0].value=fp_mul(osc[0].value,oscMix);
    osc[1].value=fp_mul(osc[1].value,255-oscMix);

}


