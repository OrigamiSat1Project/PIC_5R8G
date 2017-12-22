/*MAX2828.h*/

#ifndef __MAX2828_h__
#define __MAX2828_h__

#define LO	0
#define HI	1

#define MAX2828_DIN		RA2
#define MAX2828_SCLK	RA1
#define MAX2828_nCS		RA0
#define MAX2828_LD		RB5
#define MAX2828_EN		RB4
#define MAX2828_TXEN	RA3

#define PA_SW			RA4
#define Mod_SW			RC0

#define REGTBL_NUM		13	//レジスタテーブル数
#define REGDATA_NUM		14	//データのビット数
#define ADDRESS_NUM		4	//アドレスのビット数

//プロトタイプ宣言
void init_max2828(void);
void max2828_txon(void);

#endif	//#ifndef __MAX2828_h__