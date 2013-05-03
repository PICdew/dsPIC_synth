#include "p33fj128gp804.h"
#include "dsp.h"
#include "..\h\adcdacDrv.h"
                    
unsigned long ting;
float fs = 44212;

static int enc_states[]={0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
static int ab[]={0,0,0,0};

void initPic(void){
	// Configure Oscillator to operate the device at 40MIPS
    	// Fosc= Fin*M/(N1*N2), Fcy=Fosc/2
	// Fosc= 7.37M*43/(2*2)=79.22Mhz for ~40MIPS input clock
	PLLFBD=41;		// M=43
	CLKDIVbits.PLLPOST=0;	// N1=2
	CLKDIVbits.PLLPRE=0;                                  // N2=2
	OSCTUN=0;		// Tune FRC oscillator, if FRC is used
	RCONbits.SWDTEN=0; // (LPRC can be disabled by clearing SWDTEN bit in RCON register
	// Clock switch to incorporate PLL
	__builtin_write_OSCCONH(0x01);				// Initiate Clock Switch to FRC with PLL (NOSC=0b001)
	__builtin_write_OSCCONL(0x01);				// Start clock switching
	while (OSCCONbits.COSC != 0b001);			// Wait for Clock switch to occur
	while(OSCCONbits.LOCK!=1) {};                           // Wait for PLL to lock
}

void initDMA(void){

}

void initDac(void){
	/* Initiate DAC Clock */
	ACLKCONbits.SELACLK = 0;		// FRC w/ Pll as Clock Source
	ACLKCONbits.AOSCMD = 0;			// Auxiliary Oscillator Disabled
	ACLKCONbits.ASRCSEL = 0;		// Auxiliary Oscillator is the Clock Source
	ACLKCONbits.APSTSCLR = 7;               // FRC divide by 1
        DAC1STATbits.RITYPE = 1;                // Interrupt when FIFO is emtpy
        IPC19bits.DAC1RIP = 4;                  // DAC R interrupt has highest priority
        IFS4bits.DAC1RIF = 0;                   /* Clear Right Channel Interrupt Flag */
        IEC4bits.DAC1RIE = 1;                   /* Right Channel Interrupt Enabled */
	DAC1STATbits.ROEN = 1;			// Right Channel DAC Output Enabled
	DAC1DFLT = 0x8000;			// DAC Default value is the midpoint 		    	  // 103.16KHz  	// 8.038KHz		// 44.211KHz	// 25KHz
	DAC1CONbits.DACFDIV = 13; 	//13!! /6	//76; 			//13; 		    // 23; //		// Divide High Speed Clock by DACFDIV+1
	DAC1CONbits.FORM = 0;			// Data Format is unsigned integer
	DAC1CONbits.AMPON = 0;			// Analog Output Amplifier is enabled during Sleep Mode/Stop-in Idle mode
	DAC1CONbits.DACEN = 1;			// DAC1 Module Enabled


       
}

unsigned long phaseinc(float freq){
 return 65536*(freq/fs);

}

void digiWrite(int pin, int value){
    switch(pin){
        case 1: LATCbits.LATC3 = value; break;
        case 2: LATCbits.LATC4 = value; break;
        case 3: LATCbits.LATC5 = value; break;
        case 4: LATBbits.LATB5 = value; break;
        case 5: LATBbits.LATB6 = value; break;
        case 6: LATBbits.LATB7 = value; break;
    }
}

unsigned int digiRead(int pin){
    int value;
    switch(pin){
     //   case 1: value = PORTAbits.RA10; break;
       // case 2: value = PORTAbits.RA7; break;
    //    case 3: value = PORTAbits.RA8; break;
      //  case 4: value = PORTBbits.RB4; break;
     //   case 5: value = PORTAbits.RA4; break;
      //  case 6: value = PORTAbits.RA9; break;
    }
    return(value);
}

unsigned int anRead(int pin){
    int value;
    AD1CHS0=pin;
    AD1CON1bits.ADON=1;
    AD1CON1bits.SAMP=1;
    while(!AD1CON1bits.DONE);
    value = ADC1BUF0;
    return(value);
}

void initTimer1( void ){
IFS0bits.T1IF = 0;
T1CONbits.TCKPS = 3;  //1:1, 8 64 256
T1CONbits.TCS = 0;  //From system clock
PR1 = 43164;
IEC0bits.T1IE = 1;
T1CONbits.TON=0;
 /* enable Timer 1 and start the count */
}

void initTimer2(void){
    T2CONbits.T32=0;  // 16bit timer mode
    T2CONbits.TCKPS = 2; // /8 prescaler
    T2CONbits.TCS = 0; //Clock from system
    PR2 = 31; //Set period
    IEC0bits.T2IE = 1; //Enable interrupt
    T2CONbits.TON=0; //Start timer 2
}

void pinConfig(void){
    //PORT direction registers
    TRISA= 0b0000000000011111;
    TRISB= 0b0111111111111111;
    TRISC= 0b0000001111001111;

    //ADC config
    AD1PCFGL=0b0001111111111111;
    AD1CON1=0b0001000011100000;
    AD1CON2=0;
    AD1CON3=0xF01;
    AD1CSSL=0;

    //Encoder pullups
    CNPU1bits.CN1PUE=1;
    CNPU1bits.CN15PUE=1;
    CNPU2bits.CN27PUE=1;
    CNPU2bits.CN23PUE=1;
    CNPU2bits.CN24PUE=1;
    CNPU2bits.CN22PUE=1;
    CNPU2bits.CN21PUE=1;
    CNPU2bits.CN16PUE=1;

        LATAbits.LATA8=1;
        LATAbits.LATA9=1;
        LATAbits.LATA10=1;
        LATBbits.LATB4=1;
        LATBbits.LATB5=1;
        LATBbits.LATB10=1;
        LATBbits.LATB11=1;

}

int read_enc(int which){

    ab[which]<<=2;
    ab[which]|=((PORTB>>((which*2)+2))&0x03);
    return enc_states[ab[which]&0x0f];
}

void setColour(int which){
    switch(which){
        case 0: LATAbits.LATA8=0; LATAbits.LATA9=1; LATAbits.LATA10=1; break;
        case 1: LATAbits.LATA8=1; LATAbits.LATA9=0; LATAbits.LATA10=1; break;
        case 2: LATAbits.LATA8=1; LATAbits.LATA9=1; LATAbits.LATA10=0; break;
        case 3: LATAbits.LATA8=1; LATAbits.LATA9=1; LATAbits.LATA10=1; break;
        case 4: LATAbits.LATA8=0; LATAbits.LATA9=0; LATAbits.LATA10=0; break;

    }}
