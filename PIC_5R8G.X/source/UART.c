#include <xc.h>
//#include "pic16f886.h"
//#include <stdlib.h>

//#include "typedefine.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"
//#include "Main.h"
#include "MAX2828.h"
#include "InitMPU.h"


//変数の宣言
static UBYTE send_buf[6];	//送信用バッファ
static UBYTE rData[12];		//受信用バッファ
static USLONG dlength;		//データ長

extern UDWORD               g_data_adr;	//FROM上のアドレス(データ用)

//static bank2 volatile CamDataBuf	Rbuf2;	//画像データ用バッファ
//static bank3 volatile CamDataBuf	Rbuf3;	//画像データ用バッファ


const UBYTE STR[] = {"ABCDEFGH\r\n"};

//関数の宣言
//static void		initbau(void);
void            initbau(UBYTE);
//UBYTE    getUartData(void);
void            sendChar(UBYTE);
void            send_01(void);
void            send_OK(void);
void            send_NG(void);

//  Switch on power of amplifier
void onAmp(void){
    CAMERA_POW = 0;
    CAMERA_SEL = 0;
    max2828_txon();
	__delay_ms(10);
}

//  Switch off power of amplifier
void offAmp(void){
    CAMERA_POW = 1;
    CAMERA_SEL = 1;
    MAX2828_TXEN = 0;
    PA_SW = 0;
	__delay_ms(10);
}

////画像データ保存
//void savePicData(void)
//{
//	UBYTE idCount1 = 0;			//パッケージID(LSB)
//	UBYTE idCount2 = 0;			//パッケージID(MSB)
//	UWORD i = 0;
////	UBYTE Ret;
//	UDWORD w_adr;
//
//	w_adr = g_data_adr;
//	//ACK0を送信バッファにロード
//	for(i=0;i<6;i++)
//	{
//		send_buf[i] = ACK0[i];
//	}
//
//	while(dlength > 122)
//	{
//		//IDカウント加算
//		send_buf[4] = idCount1;
//		send_buf[5] = idCount2;
//
//		sendAckID(send_buf);		//ACK(データ取得ID)送信
//
//		//画像データ受信
//		for(i=0;i<64;i++)
//		{
//			Rbuf2.Data[i] = getUartData();
//		}
//
//		for(i=0;i<64;i++)
//		{
//			Rbuf3.Data[i] = getUartData();
//		}
//
//		saveCamPack(&w_adr);
//
//		dlength -= 122;
//
//		//イメージデータIDの加算処理
//		if(idCount1 == 0xFF)	//パッケージIDのLSBがFFなら桁上がり
//		{
//			idCount1 = 0;
//			idCount2++;
//		}
//		else
//		{
//			idCount1++;
//		}
//	}
//
//	/***最終パッケージの処理***/
//	if(dlength != 0)
//	{
//		//IDカウント加算
//		send_buf[4] = idCount1;
//		send_buf[5] = idCount2;
//
//		sendAckID(send_buf);		//ACK(データ取得ID)送信
//
//		if(dlength > 58)
//		{
//			for(i=0;i<64;i++)
//			{
//				Rbuf2.Data[i] = getUartData();
//			}
//			dlength = dlength - 60;
//
//			for(i=0;i<(dlength+2);i++)
//			{
//				Rbuf3.Data[i] = getUartData();
//			}
//
//			for(i=(dlength+2);i<64;i++)
//			{
//				Rbuf3.Data[i] = 0xFF;
//			}
//			saveCamPack(&w_adr);
//		}
//		else if(dlength == 58)
//		{
//			for(i=0;i<64;i++)
//			{
//				Rbuf2.Data[i] = getUartData();
//			}
//
//			for(i=0;i<64;i++)
//			{
//				Rbuf3.Data[i] = 0xFF;
//			}
//
//			saveCamPack(&w_adr);
//		}
//		else
//		{
//			for(i=0;i<(dlength+6);i++)
//			{
//				Rbuf2.Data[i] = getUartData();
//			}
//			for(i=(dlength+6);i<64;i++)
//			{
//				Rbuf2.Data[i] = 0xFF;
//			}
//
//			for(i=0;i<64;i++)
//			{
//				Rbuf3.Data[i] = 0xFF;
//			}
//
//			saveCamPack(&w_adr);
//		}
//
//		sendCom(AckEnd);
//	}
//	else
//	{
//		sendCom(AckEnd);
//	}
//}

//UARTの初期化
void initbau(UBYTE bau)
{
	/*ボーレート設定*/
	BRGH    = Bit_High;		//ボーレート高速モード
	BAUDCTL = 0x08;			//16倍速
	BAULATE = bau;
	SPEN    = Bit_High;		//シリアルポート設定
}

//1Byte送信関数
void sendChar(UBYTE c)
{
	while(TXIF != 1);
	TXREG = c;
}

void send_tst_str(void){
    CREN = Bit_Low;
    TXEN = Bit_High;
	UINT clock_in_tst = 0;
//	initbau(BAU_HIGH);								//UARTの初期化 115.2kbps
//	initbau(0x1F);									//57.6kbps
//	initbau(0x5F);									//19.2kbps
    CAMERA_POW = 0;
    CAMERA_SEL = 0;
    max2828_txon();
    delay_ms(1000);
	while(1){
        if(CAM2 == 0){
            if(clock_in_tst <= 1200 ){
                for(UINT i=0;i<10;i++){
                    send_buf[0] = STR[i];
                    sendChar(send_buf[0]);
                    //sendChar(0x00);
                    __delay_us(20);
                }
                if(CAM2 == 1 && CAMERA_POW == 0){
                    //  shut down power of amp
                    CLRWDT();
                    WDT_CLK = ~WDT_CLK;
                    clock_in_tst = 1300;    //  over 1200
                    CAMERA_POW = 1;
                    CAMERA_SEL = 1;
                    MAX2828_TXEN = 0;
                    PA_SW = 0;
                    break;
                }
            }else if(clock_in_tst > 1200 ){
                //  shut down power of amp
                CLRWDT();
                WDT_CLK = ~WDT_CLK;
                CAMERA_POW = 1;
                CAMERA_SEL = 1;
                MAX2828_TXEN = 0;
                PA_SW = 0;
                delay_ms(5000);
                if(CAM2 == 0 && CAMERA_POW == 1){
                    //  wake up power for amp
                    CAMERA_POW = 0;
                    CAMERA_SEL = 0;
                    max2828_txon();
                    delay_ms(1000);
                }
                clock_in_tst = 0;
            }
            clock_in_tst ++;
        }else{
            //  if amp power on
            if(CAMERA_POW == 0){
               CLRWDT();
                WDT_CLK = ~WDT_CLK;
                clock_in_tst = 0;
                CAMERA_POW = 1;
                CAMERA_SEL = 1;
                MAX2828_TXEN = 0;
                PA_SW = 0;
                clock_in_tst = 0;
            }
        }
	}
}

void send_01(void){
    UBYTE Preamblecount = 0;
    for(Preamblecount=0 ;Preamblecount <100; Preamblecount++){
        sendChar('0');
        __delay_us(20);
        sendChar('1');
        __delay_us(20);
    }
    sendChar('\r');
    sendChar('\n');
}

void send_OK(void){
    CREN = Bit_Low;
    TXEN = Bit_High;
    __delay_ms(10);
    sendChar('O');
    __delay_us(20);
    sendChar('K');
    __delay_us(20);
    sendChar('\r');
    sendChar('\n');
}

void send_NG(void){
    sendChar('N');
    __delay_us(20);
    sendChar('G');
    __delay_us(20);
}

void echo_back(void){
    UBYTE testbuf1;
    UBYTE testbuf2;
    UDWORD echo_adr = g_data_adr;
    CREN = Bit_High;
    TXEN = Bit_High;
    __delay_ms(1000);
    //flash_Erase(g_data_adr,S_ERASE);    //g_data_adrのsector65536byte分削除
    while(1){
        while(RCIF != 1);
        testbuf1 = RCREG;
        //flash_Write_Data(echo_adr,1UL,&testbuf1);
        //flash_Read_Data(echo_adr,1UL,&testbuf2);
        sendChar(testbuf1);
        echo_adr += 1UL;
    }
}

void send_dummy_data(void){
    CREN = Bit_Low;
    TXEN = Bit_High;
	UINT clock_in_tst = 0;
    delay_ms(1000);
    while(CAM2 == 0){
        if(clock_in_tst <= 1200 ){
            for(UINT i=0;i<10;i++){
                send_buf[0] = STR[i];
                sendChar(send_buf[0]);
                //sendChar(0x00);
                __delay_us(20);
            }
        }else if(1200 < clock_in_tst && clock_in_tst <= 1800){
            //  shut down power of amp
			if(CAMERA_POW == 0){
				offAmp();
			}
            __delay_ms(8);
        }else{
            clock_in_tst = 0;
            onAmp();
        }
        if(clock_in_tst % 100 == 0){
            CLRWDT();
            WDT_CLK = ~WDT_CLK;
        }
        clock_in_tst ++;
    }
}
