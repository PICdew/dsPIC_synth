#include "p33fj128gp804.h"
#include "..\h\midi.h"
#include <math.h>

#define Fcy (79227500/2)
#define BAUDRATE 31250
#define BRGVAL ((Fcy/BAUDRATE)/16)-1


void initMidi(void){


    U1MODEbits.UARTEN=0; //Disable until configured
    U1STAbits.UTXEN=0;
    U1STAbits.URXISEL=0; //Interrupt when any char is receieved
    IPC2bits.U1RXIP = 2; //USART1 Rx interrupt has highest priority
    U1BRG = BRGVAL;  //Set divisor for baudrate
    //U1MODEbits.ABAUD = 0;  //Disable auto-baud
    U1MODEbits.UARTEN=1; //Enable UART Rx
    //U1STABITS.UTXEN=1;  //Enable UART Tx
    RPINR18bits.U1RXR=2;  // Rx pin mapped to RP2
    //RPOR10bits.RP20R=3;  //Tx pin mapped tp RP20.
    IEC0bits.U1RXIE = 1;

}

