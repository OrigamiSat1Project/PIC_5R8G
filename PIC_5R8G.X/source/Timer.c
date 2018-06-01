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
 * Max count = 86.8384u * 16 = 1.389m sec with postscaler(1:1 to 16)
 * We have to check it out how long time we need
 *      1. Discard uncorrect Command
 *      2. Rest time (maybe 5s)
 * We can't send variables from main to intterupt, may have to define Command in not main but global
 * =============================================================================
 * Code
 * =============================================================================
 * void init_Interrput(void){
 *      INTCON  = 0xC0;
 *      T1CON   =
 *      TMRH    = 0x00;
 *      TMRL    = 0x00;
 *      T2CON   =
 *      TMR2    = 0x00;
 *      PR2     = 
 * }
 * 
 * void interrupt TImer_interrput(void){
 *      if(PIR1bits.TMR1IF){
 *          PIR1bits.TMR1IF = 0;
 *      }
 *      else if(PIR1bits.TMR2IF){
 *          PIR1bits.TMR2IF = 0;
 *      }
 * }
 * =============================================================================
 */