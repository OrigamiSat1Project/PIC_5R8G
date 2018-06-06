#include <xc.h>
//#include "pic16f886.h"

#include "stdio.h"
#include "string.h"
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"
#include "SECTOR.h"
#include "ReceiveJPEG.h"
#include "Downlink.h"
#include "CRC16.h"
#include "typedefine.h"
#include "Timer.h"

/* Comment
 * =============================================================================
 * Timer processing
 * If received uncorrect Command, for CIB, clear all Command.
 * 1count = 1/(7370000) * 4 = 0.54274u sec without prPrescaler, Fosc = 7370000
 * Timer2
 * Postscaler : 1:1 to 1:16
 * Prescaler : 1:1, 1:4, 1:16
 * We have to check it out how long time we need
 *      1. Discard uncorrect Command
 *      2. Rest time (maybe 5s)
 * =============================================================================
 * Code
 * =============================================================================
 */
 void initInterrupt(void){
     INTCONbits.GIE     = Bit_High;
     INTCONbits.PEIE    = Bit_High;
     PIE1bits.TMR2IE    = Bit_High;
     PIR1bits.TMR2IF    = Bit_Low;
     T2CON   = 0x3c;        
     TMR2    = 0x00;
     PR2     = 0xe6;
 }
 void interrupt incrementTimer(void){
     if(PIR1bits.TMR2IF){
         PIR1bits.TMR2IF = 0;
         TMR2 = 0x00;
         timer_counter++;
     }
     if(timer_counter == 15000){
         timer_counter = 0;
     }
 }