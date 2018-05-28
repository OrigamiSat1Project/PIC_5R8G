/*** ãƒã‚¤ã‚³ãƒ³ã®IOãƒã?¼ãƒˆè¨­å®? ***/

#include <xc.h>
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"

/*** ãƒã‚¤ã‚³ãƒ³åˆæœŸåŒ–å?¦ç? ***/
void init_mpu(void)
{
	//ãƒã?¼ãƒˆã?®åˆæœŸåŒ?
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;	
	
	//ADè¨­å®šï¼ˆå?¨ã¦ãƒ?ã‚¸ã‚¿ãƒ«å…¥åŠ›ï¼?
	ANSEL  = 0x00;	//ADè¨­å®?
	ANSELH = 0x00;	//ADè¨­å®?
	
	//ãƒã?¼ãƒˆå?¥å‡ºåŠ›è¨­å®?	
	TRISA  = 0xC0;	//å…¥å‡ºåŠ›è¨­å®?
	TRISB  = 0x2B;	//å…¥å‡ºåŠ›è¨­å®?
    	TRISC  = 0x84;	//å…¥å‡ºåŠ›è¨­å®?
	
	//ãƒã?¼ãƒˆå?æœŸå€¤è¨­å®?		
	PORTA  = 0x21;	//åˆæœŸå€¤è¨­å®?
	PORTB  = 0x94;	//åˆæœŸå€¤è¨­å®?
	PORTC  = 0x41;	//åˆæœŸå€¤è¨­å®?
}

void init_module(void){
    init_mpu();
    initbau(BAU_HIGH);                //115200bps
    //initbau(BAU_MIDDLE);              //57600bps
    //initbau(BAU_LOW);                   //14400bps
    MAX2828_EN = 1;                     //MAX2828 ON
    __delay_us(100);                    //100us wait
    FLASH_SPI_EI();                     //enable SPI
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
	
