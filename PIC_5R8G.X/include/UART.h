/*UART.h*/

#ifndef __UART_h__
#define __UART_h__

#include "typedefine.h"

//ボーレート設定
#define BAULATE		SPBRG		//ボーレート設定レジスタ
#define BAU_LOW		0x7F		//14.4kbps
#define BAU_MIDDLE  0x1F		//57.6kbps
#define BAU_HIGH	0x0F		//115.2kbps

//TODO : this shoule be optimisation 
#define BAU_WITH_OBC BAU_LOW

#define cam1		0x00
#define cam2		0x01

#define CAMERA_SEL	RB7
#define CAMERA_POW	RC1
#define BUSY		RA5
#define SEND		RC2

#define Bit_High	0b1
#define Bit_Low		0b0

//柏山が定義
#define Bank0 STATUS = 0x00;
#define Bank1 STATUS = 0x20;
#define Bank2 STATUS = 0xC0;
#define Bank3 STATUS = 0xE0;

//static UBYTE getUartData(void);

void send_tst_str(void);
void initbau(UBYTE);
void sendChar(UBYTE);
void send_01(void);
void send_AB(void);
void send_OK(void);
void send_NG(void);
void send_CRLF(void);
void echo_back(void);
void send_dummy_data(void);
UBYTE    getUartData(UBYTE);
typedef union{
	UBYTE	Data[64];
	UDWORD	dummy;
}CamDataBuf;

void onAmp(void);
void offAmp(void);
void change_downlink_baurate(UBYTE);
UBYTE getDownlinkBAU(void);

//extern bank2 volatile CamDataBuf	Rbuf2;		//画像データ用バッファ
//extern bank3 volatile CamDataBuf	Rbuf3;		//画像データ用バッファ
//extern USLONG 						dlength;	//データ長


#endif						//#ifndef __UART_h__
