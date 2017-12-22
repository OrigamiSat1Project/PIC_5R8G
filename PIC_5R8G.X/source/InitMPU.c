/*** マイコンのIOポート設定 ***/

#include <xc.h>
//#include "pic16f886.h"
#include "InitMPU.h"

/*** マイコン初期化処理 ***/
void init_mpu(void)
{
	//ポートの初期化
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;	
	
	//AD設定（全てデジタル入力）
	ANSEL  = 0x00;	//AD設定
	ANSELH = 0x00;	//AD設定
	
	//ポート入出力設定	
	TRISA  = 0xC0;	//入出力設定
	TRISB  = 0x2B;	//入出力設定
    TRISC  = 0x84;	//入出力設定
	
	//ポート初期値設定		
	PORTA  = 0x21;	//初期値設定
	PORTB  = 0x94;	//初期値設定
	PORTC  = 0x41;	//初期値設定
	
}
