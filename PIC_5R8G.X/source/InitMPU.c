/*** �}�C�R����IO�|�[�g�ݒ� ***/

#include <xc.h>
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"

/*** �}�C�R������������ ***/
void init_mpu(void)
{
	//�|�[�g�̏�����
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;	
	
	//AD�ݒ�i�S�ăf�W�^�����́j
	ANSEL  = 0x00;	//AD�ݒ�
	ANSELH = 0x00;	//AD�ݒ�
	
	//�|�[�g���o�͐ݒ�	
	TRISA  = 0xC0;	//���o�͐ݒ�
	TRISB  = 0x2B;	//���o�͐ݒ�
    TRISC  = 0x84;	//���o�͐ݒ�
	
	//�|�[�g�����l�ݒ�		
	PORTA  = 0x21;	//�����l�ݒ�
	PORTB  = 0x94;	//�����l�ݒ�
	PORTC  = 0x41;	//�����l�ݒ�
}

void init_module(void){
    init_mpu();
    initbau(BAU_HIGH);                //115200bps
    //initbau(BAU_MIDDLE);              //57600bps
    //initbau(BAU_LOW);                   //14400bps
    MAX2828_EN = 1;                     //MAX2828 ON
    __delay_us(100);                    //100us wait
    FLASH_SPI_EI();                     //enable SPI
    //FIXME for debug by simulator
    //init_max2828();                     //init MAX2828
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
	