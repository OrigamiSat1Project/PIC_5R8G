/*** 繝槭�?繧?��繝ｳ縺?��IO繝昴??��繝郁?��?��螳? ***/

#include <xc.h>
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"
//#include "Timer.h"

/*** 繝槭�?繧?��繝ｳ蛻�?�?蛹�???���?? ***/
void init_mpu(void)
{
	//繝昴??��繝医??��蛻�?�?蛹?
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;	
	
	//AD險?��螳夲?��亥??��縺?���??繧?��繧?��繝ｫ蜈･蜉幢?��?
	ANSEL  = 0x00;	//AD險?��螳?
	ANSELH = 0x00;	//AD險?��螳?
	
	//繝昴??��繝亥??���?��蜉�?��?��螳?	
	TRISA  = 0xC0;	//蜈･�?��蜉�?��?��螳?
	TRISB  = 0x2B;	//蜈･�?��蜉�?��?��螳?
    	TRISC  = 0x84;	//蜈･�?��蜉�?��?��螳?
	
	//繝昴??��繝亥?�?�?蛟､險?��螳?		
	PORTA  = 0x21;	//蛻�?�?蛟､險?��螳?
	PORTB  = 0x94;	//蛻�?�?蛟､險?��螳?
	PORTC  = 0x41;	//蛻�?�?蛟､險?��螳?
}

void init_module(void){
    init_mpu();
    initbau(BAU_WITH_OBC);                   //14400bps
    MAX2828_EN = 1;                     //MAX2828 ON
    __delay_us(100);                    //100us wait
    FLASH_SPI_EI();                     //enable SPI
    init_max2828();                     //init MAX2828
    Mod_SW = 0;                         //FSK modulation ON
    //XXX : Timer
    //FIXME : debug
    //initInterrupt();
}
    
    /* Comment
     * ======================================================================================
     * Make initialize mode
        Only copy the first regulation above and paste
     * ======================================================================================
     * Code
     * ======================================================================================
     *   init_mpu();
     *   //initbau(BAU_HIGH);                //115200bps
     *   //initbau(BAU_MIDDLE);              //57600bps
     *   initbau(BAU_LOW);                   //14400bps
     *   MAX2828_EN = 1;                     //MAX2828 ON
     *    __delay_us(100);                    //100us wait
     *    FLASH_SPI_EI();                     //enable SPI
     *   init_max2828();                     //init MAX2828
     *   Mod_SW = 0;                         //FSK modulation ON
     * ======================================================================================
     */