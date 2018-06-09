/*** ç¹æ§­ã?ç¹§?½³ç¹ï½³ç¸º?½®IOç¹æ˜´??½¼ç¹éƒ?½¨?½­è³? ***/

#include <xc.h>
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"
//#include "Timer.h"

/*** ç¹æ§­ã?ç¹§?½³ç¹ï½³è›»æ™?æ‚?è›¹é–???½¦é€?? ***/
void init_mpu(void)
{
	//ç¹æ˜´??½¼ç¹åŒ»??½®è›»æ™?æ‚?è›¹?
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;	
	
	//ADéšª?½­è³å¤²?½¼äº¥??½¨ç¸º?½¦ç¹??ç¹§?½¸ç¹§?½¿ç¹ï½«èœˆï½¥èœ‰å¹¢?½¼?
	ANSEL  = 0x00;	//ADéšª?½­è³?
	ANSELH = 0x00;	//ADéšª?½­è³?
	
	//ç¹æ˜´??½¼ç¹äº¥??½¥èœ?½ºèœ‰å¹?½¨?½­è³?	
	TRISA  = 0xC0;	//èœˆï½¥èœ?½ºèœ‰å¹?½¨?½­è³?
	TRISB  = 0x2B;	//èœˆï½¥èœ?½ºèœ‰å¹?½¨?½­è³?
    	TRISC  = 0x84;	//èœˆï½¥èœ?½ºèœ‰å¹?½¨?½­è³?
	
	//ç¹æ˜´??½¼ç¹äº¥?æ™?æ‚?è›Ÿï½¤éšª?½­è³?		
	PORTA  = 0x21;	//è›»æ™?æ‚?è›Ÿï½¤éšª?½­è³?
	PORTB  = 0x94;	//è›»æ™?æ‚?è›Ÿï½¤éšª?½­è³?
	PORTC  = 0x41;	//è›»æ™?æ‚?è›Ÿï½¤éšª?½­è³?
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