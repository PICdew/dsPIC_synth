
#ifndef __ADCDRV1_H__
#define __ADCDRV1_H__ 

// Sampling Control
#define Fosc	79227500
#define Fcy	(Fosc/2)

// Functions
void initPic(void);
void initDac(void);
unsigned long phaseinc(float freq);
void digiWrite(int pin, int value);
unsigned int digiRead(int pin);
unsigned int anRead(int pin);
void setColour(int which);
void __attribute__((interrupt, no_auto_psv)) _DAC1RInterrupt(void);
void __attribute__((interrupt, auto_psv)) _T1Interrupt( void );
void __attribute__((interrupt, auto_psv)) _T2Interrupt( void );
#endif

