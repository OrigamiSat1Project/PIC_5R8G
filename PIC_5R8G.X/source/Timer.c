#include <xc.h>
//#include "pic16f886.h"

#include "typedefine.h"
#include "Timer.h"
#include <limits.h>

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
 */

static UINT timer_counter = 0;
static UINT timer_counter_min = 0;
static UINT timer_counter_only_getUart = 0;
static UINT timer_counter_only_rest = 0;

void initInterrupt(void){
    INTCONbits.GIE  = 1;
    INTCONbits.PEIE = 1;
    PIE1bits.TMR2IE = 1;
    PIR1bits.TMR2IF = 0;
    T2CON   = 0x3c;
    TMR2    = 0x00;
    PR2     = 0xe6;
}
void interrupt incrementTimer(void){
    if(PIR1bits.TMR2IF){
        PIR1bits.TMR2IF = 0;
        TMR2 = 0x00;
        timer_counter++;
        timer_counter_only_rest++;
        timer_counter_only_getUart++;
    }
    if(timer_counter == 60000){
        timer_counter = 0;
        timer_counter_min++;
    }
    if(timer_counter == UINT_MAX){
        timer_counter = 0;
    }
    if(timer_counter_min == UINT_MAX){
        timer_counter_min = 0;
    }
    if(timer_counter_only_getUart == UINT_MAX){
        timer_counter_only_getUart = 0;
    }
    if(timer_counter_only_rest == UINT_MAX){
        timer_counter_only_rest = 0;
    }
}

void set_timer_counter(UINT time){
    timer_counter = time;
}

UINT get_timer_counter(void){
    return timer_counter;
}

void set_timer_counter_min(UINT time_min){
    timer_counter_min = time_min;
}

UINT get_timer_counter_min(void){
    return timer_counter_min;
}

void set_timer_counter_only_getUart(UINT time){
    timer_counter_only_getUart = time;
}

UINT get_timer_counter_only_getUart(void){
    return timer_counter_only_getUart;
}

void set_timer_counter_only_rest(UINT time){
    timer_counter_only_rest = time;
}

UINT get_timer_counter_only_rest(void){
    return timer_counter_only_rest;
}