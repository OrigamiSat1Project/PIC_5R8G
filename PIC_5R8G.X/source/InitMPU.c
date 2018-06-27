#include <xc.h>
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"
#include "Timer.h"


void init_mpu(void)
{
	
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;

	
	ANSEL  = 0x00;	
	ANSELH = 0x00;	


	TRISA  = 0xC0;	
	TRISB  = 0x2B;	
    TRISC  = 0x84;

	
	PORTA  = 0x21;
	PORTB  = 0x94;
	PORTC  = 0x41;
}

void init_module(void){
    init_mpu();
    initbau(BAU_WITH_OBC);                   //14400bps
    MAX2828_EN = 1;                     //MAX2828 ON
    __delay_us(100);                    //100us wait
    FLASH_SPI_EI();                     //enable SPI
    init_max2828();                     //init MAX2828
    Mod_SW = 0;                         //FSK modulation ON
    initInterrupt();
}
