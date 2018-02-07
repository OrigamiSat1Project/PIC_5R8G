/*UART.h*/

#ifndef __UART_h__
#define __UART_h__

#include "typedefine.h"

//�{�[���[�g�ݒ�
#define BAULATE		SPBRG		//�{�[���[�g�ݒ背�W�X�^
#define BAU_LOW		0x7F		//14.4kbps
#define BAU_HIGH	0x0F		//115.2kbps

#define cam1		0x00
#define cam2		0x01

#define CAMERA_SEL	RB7
#define CAMERA_POW	RC1
#define BUSY		RA5
#define SEND		RC2

#define Bit_High	0b1
#define Bit_Low		0b0

//���R����`
#define Bank0 STATUS = 0x00;
#define Bank1 STATUS = 0x20;
#define Bank2 STATUS = 0xC0;
#define Bank3 STATUS = 0xE0;

//static UBYTE getUartData(void);

//�֐��̐錾
/* */
void setupCam(UBYTE);
//void getPicture(void);
void getPicSize(void);
void getPicData(void);	
UBYTE CheckSendPort(void);
void savePicData(void);
UBYTE savePicSize(void);
void sendModData(UDWORD);
void send_pn9(void);
void send_55(void);

void send_tst_str(void);
void initbau(UBYTE);
void sendChar(UBYTE);
void send_01(void);
void send_OK(void);
void send_NG(void);
void echo_back(void);
UBYTE    getUartData(void);
typedef union{
	UBYTE	Data[64];
	UDWORD	dummy;
}CamDataBuf;

//extern bank2 volatile CamDataBuf	Rbuf2;		//�摜�f�[�^�p�o�b�t�@
//extern bank3 volatile CamDataBuf	Rbuf3;		//�摜�f�[�^�p�o�b�t�@
//extern USLONG 						dlength;	//�f�[�^��


#endif						//#ifndef __UART_h__