/*** マイコンのIOポ�?�ト設�? ***/

#include <xc.h>
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"

/*** マイコン初期化�?��? ***/
void init_mpu(void)
{
	//ポ�?�ト�?�初期�?
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;	
	
	//AD設定（�?�て�?ジタル入力�?
	ANSEL  = 0x00;	//AD設�?
	ANSELH = 0x00;	//AD設�?
	
	//ポ�?�ト�?�出力設�?	
	TRISA  = 0xC0;	//入出力設�?
	TRISB  = 0x2B;	//入出力設�?
    	TRISC  = 0x84;	//入出力設�?
	
	//ポ�?�ト�?�期値設�?		
	PORTA  = 0x21;	//初期値設�?
	PORTB  = 0x94;	//初期値設�?
	PORTC  = 0x41;	//初期値設�?
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
	
