/*** MAX2828の制御 ***/

#include <xc.h>
#include "pic16f886.h"

#include "MAX2828.h"
#include "typedefine.h"

//定数の宣言
const UWORD MAX2828_REG_INIT_TBL[REGTBL_NUM] = {
/*REGISTER	|					DEFAULT																*/
/*				D13	D12	D11	D10	D9	D8	D7	D6	D5	D4	D3	D2	D1	D0	|	D[13:0](hex)|	A[3:0]	*/	
/*	Register0 	0 	1 	0 	0 	0 	1 	0 	1 	0 	0 	0 	0 	0 	0 	*/	0x1140,		/*	0		*/
/*	Register1 	0	0 	0 	0 	0 	0 	1 	1 	0 	0 	1 	0 	1 	0 	*/	0x00CA,		/*	1		*/
/*	Standby 	0 	1 	0 	0 	0 	0 	0 	0 	0 	0 	0 	1 	1 	1 	*/	0x1007,		/*	2	 	*/
/*Integer-Divider																					*/	
/*Ratio			0 	1 	0 	0 	0 	0 	1 	1 	1 	0 	1 	0	0 	1 	*/	0x10E9,		/*	3		*/
/*Fractional-																						*/	
/*Divider Ratio	1 	0 	0 	1 	1 	0 	0 	1 	1 	0 	0 	1 	1 	0 	*/	0x2666,		/*	4		*/
/*Band Select																						*/	
/*and PLL		0 	1 	1 	0 	0 	0 	1 	1 	1 	0 	0 	0 	1 	1 	*/	0x18E3,		/*	5		*/
/*Calibration 	0 	1 	1 	1 	0 	0 	0 	0 	0 	0 	0 	0 	0 	0 	*/	0x1C00,		/*	6		*/
/*Lowpass Filter0 	0 	0 	0 	0  	0 	0 	0 	1 	0 	1 	0 	1 	0 	*/	0x002A,		/*	7		*/
/*Rx																								*/	
/*Control/RSSI	0 	1 	0 	0 	0 	0 	0 	0 	1	0	0 	1 	0 	1 	*/	0x1025,		/*	8		*/
/*Tx																								*/	
/*Linearity/Baseband																				*/	
/*Gain			0 	0 	0 	1 	1 	0 	0 	0 	0 	0 	0 	0	0 	0 	*/	0x0600,		/*	9		*/
/*PA Bias DAC	0 	0	0 	0 	1 	1 	1 	1 	0 	0 	0 	0 	0 	0 	*/	0x03C0,		/*	10		*/
/*Rx Gain 		0 	0 	0 	0 	0 	0 	0 	0 	0 	0 	0	0 	0 	0 	*/	0x0000,		/*	11		*/
/*Tx VGA Gain 	0 	0 	0 	0 	0 	0 	0 	0 	1 	1 	0 	0 	1 	1 	*/	0x0033,		/*	12		*/

};

//変数の宣言
UWORD	MAX2828_REGDATA[REGTBL_NUM];
static UBYTE ldcount = 0;	//ロック検出用カウンタ

//プロトタイプ宣言
static void txdata_rf(UWORD,UBYTE,UWORD,UBYTE,UBYTE);
static void init_reg_MAX2828(void);
static UBYTE check_ld(void);

//シリアルデータ送信
static void txdata_rf( UWORD data , UBYTE data_num , 
						UWORD adr  , UBYTE adr_num  , 
						UBYTE cs)
{
	UWORD mask_bit=0;
	
	MAX2828_nCS = LO;

	while(0!=data_num)
	{
		mask_bit = 1 << (data_num - 1);
		if(data & mask_bit)
			MAX2828_DIN = HI;
		else
			MAX2828_DIN = LO;
		MAX2828_SCLK = HI;
		MAX2828_SCLK = LO;
		data_num--;
		MAX2828_DIN = LO;
	}			

	while(0!=adr_num)
	{
		mask_bit = 1 << (adr_num - 1);
		if(adr & mask_bit)
			MAX2828_DIN = HI;
		else
			MAX2828_DIN = LO;
		MAX2828_SCLK = HI;
		MAX2828_SCLK = LO;
		adr_num--;
		MAX2828_DIN = LO;
	}			
	
	MAX2828_SCLK = LO;
	
  //--- チップセレクトをHI ---
	MAX2828_nCS = HI;
}

//送信データ用レジスタの設定
static void init_reg_MAX2828()
{
	UBYTE i;
	
	for(i=0;i<REGTBL_NUM;i++)
	{
		txdata_rf(MAX2828_REGDATA[i],REGDATA_NUM,i,ADDRESS_NUM,1);
	}
}

//MAX2828の初期化
void init_max2828(void)
{
	UBYTE i;

	for(i=0;i<REGTBL_NUM;i++)
	{
		MAX2828_REGDATA[i]=MAX2828_REG_INIT_TBL[i];
	}

	init_reg_MAX2828();             //MAX2828レジスタ設定
	while(!check_ld());             //ロック待ち
}

//MAX2828 TXオン
void max2828_txon(void){
//	while(!check_ld());				//ロック待ち
//	MAX2828_TXEN = HI;				//TXオン
//	RA4 = 0b1;					//TXPAオン
	PORTA |= 0x18;				//MAX2828 EN, TXPA ON
	PORTA |= 0x08;				//MAX2828 EN, TXPA OFF
}

//ロック検出用関数
static UBYTE check_ld(void)
{
	if(MAX2828_LD != LO)
	{
		if(ldcount < 30)
		{
			ldcount++;
			return 0;
		}
		else if(ldcount >= 30)
		{
			ldcount = 0;
			return 1;
		}
	}
	ldcount = 0;
	return 0;
}