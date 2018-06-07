/*** 繝槭う繧ｳ繝ｳ縺ｮIO繝昴?ｼ繝郁ｨｭ螳? ***/

#include <xc.h>
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"

/*** 繝槭う繧ｳ繝ｳ蛻晄悄蛹門?ｦ逅? ***/
void init_mpu(void)
{
	//繝昴?ｼ繝医?ｮ蛻晄悄蛹?
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;	
	
	//AD險ｭ螳夲ｼ亥?ｨ縺ｦ繝?繧ｸ繧ｿ繝ｫ蜈･蜉幢ｼ?
	ANSEL  = 0x00;	//AD險ｭ螳?
	ANSELH = 0x00;	//AD險ｭ螳?
	
	//繝昴?ｼ繝亥?･蜃ｺ蜉幄ｨｭ螳?	
	TRISA  = 0xC0;	//蜈･蜃ｺ蜉幄ｨｭ螳?
	TRISB  = 0x2B;	//蜈･蜃ｺ蜉幄ｨｭ螳?
    	TRISC  = 0x84;	//蜈･蜃ｺ蜉幄ｨｭ螳?
	
	//繝昴?ｼ繝亥?晄悄蛟､險ｭ螳?		
	PORTA  = 0x21;	//蛻晄悄蛟､險ｭ螳?
	PORTB  = 0x94;	//蛻晄悄蛟､險ｭ螳?
	PORTC  = 0x41;	//蛻晄悄蛟､險ｭ螳?
}

void init_module(void){
    init_mpu();
    initbau(BAU_WITH_OBC);                   //14400bps
    MAX2828_EN = 1;                     //MAX2828 ON
    __delay_us(100);                    //100us wait
    FLASH_SPI_EI();                     //enable SPI
    //FIXME : debug
    init_max2828();                     //init MAX2828
    Mod_SW = 0;                         //FSK modulation ON
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
	
