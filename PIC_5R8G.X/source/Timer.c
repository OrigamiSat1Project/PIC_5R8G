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
 * 1count = 1/(7370000) * 4 = 5.4274u sec without prPrescaler, Fosc = 7370000
 * Timer1
 * 1count_max = 5.4274 * 8 = 43.4192u sec with prescaler(1:1,2,4,8)
 * Max count = 43.4192u * 256 * 256 = 2.845 sec with TMR1H and TMR1L
 * Timer2
 * 1count_max = 5,4274 * 16 =86.8384u sec with prescaler(1:1,4,16)
 * count      = 86.8384u * 16 = 1.389m sec with postscaler(1:1 to 16)
 * Max count  = 1.389m * 256 = 355.384m sec
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
     //Postscaler 1:8, Prescalr 1:1 1count:5.4274u * 8 = 43.4192u
     T2CON   = 0x3c;        
     TMR2    = 0xe7;         //231 count * 43.4192us = 10.03ms
     PR2     = 0x00;       
 }
 void interrupt incrementTimer(void){
     if(PIR1bits.TMR2IF){
         PIR1bits.TMR2IF = 0;
         PR2 = 0x00;
         timer_counter++;
     }
 }