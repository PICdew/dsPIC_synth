/* 
 * File:   adsr.h
 * Author: Owner
 *
 * Created on January 31, 2013, 5:31 PM
 */

#ifndef ADSR_H
#define	ADSR_H


void noteonAdsr(adsr_t* adsr);
void noteoffAdsr(adsr_t* adsr);
void calcAdsr(adsr_t* adsr);

#endif	/* ADSR_H */

