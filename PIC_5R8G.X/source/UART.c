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

static bank2 volatile CamDataBuf	Rbuf2;	//画像データ用バッファ
static bank3 volatile CamDataBuf	Rbuf3;	//画像データ用バッファ

//定数の宣言
const UBYTE syncWord[6]   = {0xAA,0x0D,0x00,0x00,0x00,0x00};
const UBYTE ACK0[6]       = {0xAA,0x0E,0x00,0x00,0x00,0x00};
const UBYTE AckEnd[6]     = {0xAA,0x0E,0x00,0x00,0xF0,0xF0};
const UBYTE InitCam[6]    = {0xAA,0x01,0x04,0x07,0x00,0x07};
const UBYTE changeSize[6] = {0xAA,0x06,0x08,0x80,0x00,0x00};	//1パケット=128バイト
const UBYTE SnapShot[6]   = {0xAA,0x05,0x00,0x00,0x00,0x00};
const UBYTE GetPicCom[6]  = {0xAA,0x04,0x01,0x00,0x00,0x00};

const UBYTE PRBS9[64] =	   {0x21, 0x86, 0x9c, 0x6a, 0xd8, 0xcb, 0x4e, 0x14,
							0x6a, 0xf9, 0x4d, 0xd2, 0x7e, 0xb2, 0x32, 0x03,
							0xc6, 0x14, 0x4b, 0x7f, 0xd1, 0xb8, 0xa6, 0x79,
							0x7c, 0x17, 0xac, 0xed, 0x06, 0xad, 0xaf, 0x0a,
							0x94, 0x7a, 0xba, 0x03, 0xe7, 0x92, 0xd7, 0x15,
							0x09, 0x73, 0xe8, 0x6d, 0x16, 0xee, 0xe1, 0x3f,
							0x78, 0x1f, 0x9d, 0x09, 0x52, 0x6e, 0xf1, 0x7c,
							0x36, 0x2a, 0x71, 0x6c, 0x75, 0x64, 0x44, 0x80
};

const UBYTE STR[] = {"ABCDEFGH\r\n"};

//関数の宣言
//static void		initbau(void);
void            initbau(UBYTE);
UBYTE           getUartData(void);
void            sendChar(UBYTE);
void            send_01(void);
void            send_OK(void);
void            send_NG(void);
static void		syncCam(void);
static void		sendCom(const UBYTE *);
static void		changePackageSize(void);
static void		snap(void);
static USLONG	sendGetPicCom(void);
static void		sendAckID(UBYTE *);
static void		saveCamPack(UDWORD *);
static void		loadCamPack(UDWORD *);
static UBYTE	mk_pn9(void);
static void		set_pn9(void);

//  TODO : this should be tested by simulator & debug
static UBYTE BAU_WHEN_DOWNLINK = BAU_WITH_OBC;

//  Switch on power of amplifier
void onAmp(){
    initbau(BAU_WHEN_DOWNLINK);
    __delay_ms(10);
    //CAMERA_POW = 0;
    //CAMERA_SEL = 0;
    //max2828_txon();
	__delay_ms(10);
}

//  Switch off power of amplifier
void offAmp(void){
    initbau(BAU_WITH_OBC);
    __delay_ms(10);
    CAMERA_POW = 1;
    CAMERA_SEL = 1;
    MAX2828_TXEN = 0;
    PA_SW = 0;
	__delay_ms(10);
}

//*** SENDポートのチェック ***
//UBYTE return 0：未検出　、1：検出
UBYTE CheckSendPort(void)
{
	static UBYTE	cha = (UBYTE)0;

	if(SEND == 0)
	{
		if(cha < 30)
		{
			cha++;
			return 0;
		}
		else if(cha >= 30)
		{
			cha = 0;
			return 1;
		}
	}
	cha = 0;
	return 0;
}

////カメラのセットアップ
//void setupCam(UBYTE cam)
//{
//	//カメラの選択
//	if(cam)
//	{
//		CAMERA_SEL = Bit_Low;		//カメラ2を選択
//	}
//	else
//	{
//		CAMERA_SEL = Bit_High;		//カメラ1を選択
//	}
//
//	CAMERA_POW = Bit_High;			//カメラ電源ON
//
//	initbau(BAU_LOW);				//MPU UART初期設定
//	__delay_ms(1000);				//1sウェイト
//	syncCam();						//カメラとの同期
//
//	BAULATE = BAU_HIGH;				//MPUのボーレートを115.2kbpsに変更
//
//	changePackageSize();			//パッケージサイズを128BYTEに設定
//}

//画像データサイズの取得
void getPicSize(void)
{
	snap();							//撮影
	dlength = sendGetPicCom();		//データ長の取得
}

//画像データサイズの保存
UBYTE savePicSize(void)
{
	EXCHG_LONG len;
	EXCHG_LONG tmp;
	UBYTE i = 0;
	UBYTE j = 3;
	UBYTE Ret;

	len.us = (UDWORD)dlength;
	for(i=0;i<4;i++)
	{
		tmp.uc[j] = len.uc[i];
		j--;
	}
	Ret = flash_Write_Page(0UL,(UWORD)4,tmp.uc);
	return Ret;
}

//画像データ保存
void savePicData(void)
{
	UBYTE idCount1 = 0;			//パッケージID(LSB)
	UBYTE idCount2 = 0;			//パッケージID(MSB)
	UWORD i = 0;
//	UBYTE Ret;
	UDWORD w_adr;

	w_adr = g_data_adr;
	//ACK0を送信バッファにロード
	for(i=0;i<6;i++)
	{
		send_buf[i] = ACK0[i];
	}

	while(dlength > 122)
	{
		//IDカウント加算
		send_buf[4] = idCount1;
		send_buf[5] = idCount2;

		sendAckID(send_buf);		//ACK(データ取得ID)送信

		//画像データ受信
		for(i=0;i<64;i++)
		{
			Rbuf2.Data[i] = getUartData();
		}

		for(i=0;i<64;i++)
		{
			Rbuf3.Data[i] = getUartData();
		}

		saveCamPack(&w_adr);

		dlength -= 122;

		//イメージデータIDの加算処理
		if(idCount1 == 0xFF)	//パッケージIDのLSBがFFなら桁上がり
		{
			idCount1 = 0;
			idCount2++;
		}
		else
		{
			idCount1++;
		}
	}

	/***最終パッケージの処理***/
	if(dlength != 0)
	{
		//IDカウント加算
		send_buf[4] = idCount1;
		send_buf[5] = idCount2;

		sendAckID(send_buf);		//ACK(データ取得ID)送信

		if(dlength > 58)
		{
			for(i=0;i<64;i++)
			{
				Rbuf2.Data[i] = getUartData();
			}
			dlength = dlength - 60;

			for(i=0;i<(dlength+2);i++)
			{
				Rbuf3.Data[i] = getUartData();
			}

			for(i=(dlength+2);i<64;i++)
			{
				Rbuf3.Data[i] = 0xFF;
			}
			saveCamPack(&w_adr);
		}
		else if(dlength == 58)
		{
			for(i=0;i<64;i++)
			{
				Rbuf2.Data[i] = getUartData();
			}

			for(i=0;i<64;i++)
			{
				Rbuf3.Data[i] = 0xFF;
			}

			saveCamPack(&w_adr);
		}
		else
		{
			for(i=0;i<(dlength+6);i++)
			{
				Rbuf2.Data[i] = getUartData();
			}
			for(i=(dlength+6);i<64;i++)
			{
				Rbuf2.Data[i] = 0xFF;
			}

			for(i=0;i<64;i++)
			{
				Rbuf3.Data[i] = 0xFF;
			}

			saveCamPack(&w_adr);
		}

		sendCom(AckEnd);
	}
	else
	{
		sendCom(AckEnd);
	}
}

/* 変調データ送信 */
void sendModData(UDWORD RAddr)
{
	UBYTE 		Ret;
	UBYTE		i;
	UBYTE		j;
	EXCHG_LONG	len;
	EXCHG_LONG	tmp;

	initbau(BAU_HIGH);								//UARTの初期化
//	BAULATE = BAU_HIGH;						//115.2kbpsに設定
	CREN = Bit_Low;							//受信禁止

	//データ長取得
	Ret = flash_Read_Data(0UL,4,len.uc);	//データ長をFROMから読み出し
	j = 3;
	for(i=0;i<4;i++)						//エンディアンの変換
	{
		tmp.uc[i] = len.uc[j];
		j--;
	}
	dlength = (USLONG)tmp.us;

	Mod_SW = Bit_Low;						//変調ON
	max2828_txon();
	PA_SW  = Bit_High;

	delay_ms(1);

	//変調データ取得・送信
	while(dlength > 122)
	{
		loadCamPack(&RAddr);				//画像データをFROMからバッファに読み出し

		//バッファ内のデータをUART送信
		for(i=0;i<64;i++)
		{
			sendChar(Rbuf2.Data[i]);
		}
		for(i=0;i<64;i++)
		{
			sendChar(Rbuf3.Data[i]);
		}
		dlength -= 122;
	}
	//最終パケットの送信
	loadCamPack(&RAddr);
	for(i=0;i<64;i++)
	{
		sendChar(Rbuf2.Data[i]);
	}
	for(i=0;i<64;i++)
	{
		sendChar(Rbuf3.Data[i]);
	}

	PA_SW = Bit_Low;
	MAX2828_TXEN = Bit_Low;
	Mod_SW = Bit_High;						//変調OFF
}

//UARTの初期化
//static void initbau(void)
//{
//	/*ボーレート設定*/
//	BRGH    = Bit_High;		//ボーレート高速モード
//	BAUDCTL = 0x08;			//16倍速
//	BAULATE = BAU_LOW;		//14.4kbps
//	SPEN    = Bit_High;		//シリアルポート設定
//	TXEN    = Bit_High;		//送信許可
//	CREN    = Bit_High;		//受信許可
//}

//UARTの初期化
void initbau(UBYTE bau)
{
    if((bau != BAU_LOW ) &&
       (bau != BAU_MIDDLE) &&
       (bau != BAU_HIGH))
    {
            bau = BAU_WITH_OBC;
    }
	/*ボーレート設定*/
	BRGH    = Bit_High;		//ボーレート高速モード
	BAUDCTL = 0x08;			//16倍速
	BAULATE = bau;
	SPEN    = Bit_High;		//シリアルポート設定
}

//パッケージサイズ変更関数
static void changePackageSize(void)
{
	UBYTE i = 0;

	sendCom(changeSize);	//パッケージサイズを128BYTEに変更

	//ACKの空読み
	for(i=0;i<6;i++)
	{
		rData[i] = getUartData();
	}
}

//撮影コマンド送信
static void snap(void)
{
	UBYTE i = 0;

	sendCom(SnapShot);	//パッケージサイズを128BYTEに変更

	//Ackの空読み
	for(i=0;i<6;i++)
	{
		rData[i] = getUartData();
	}
}

//データ長取得
static USLONG sendGetPicCom(void)
{
	UBYTE i = 0;

	sendCom(GetPicCom);	//Jpeg圧縮コマンド送信

	for(i=0;i<12;i++)
	{
		rData[i] = getUartData();
	}

	return ((USLONG)rData[11]<<16)+((USLONG)rData[10]<<8)+(USLONG)rData[9];	//データ長の計算
}

//カメラ同期関数
static void syncCam(void)
{
	UBYTE i = 0;
	UBYTE j = 0;

	for(i = 0;i < 60;i++)
	{
		if(RCIF)	//受信バッファにデータがあるとき
		{
			for(j = 0;j < 12;j ++)
			{
				rData[j] = getUartData();
			}
			j = 0;

			sendCom(ACK0);		//ACK0送信
			delay_ms(1);

			sendCom(InitCam);	//カメラ設定(115.2kbps,VGA)
			delay_ms(1);

			//ACKの空読み
			for(j = 0;j < 6;j++)
			{
				rData[j] = getUartData();
			}

			delay_ms(50);		//50msウェイト

			return;
		}
		else
		{
			sendCom(syncWord);	//同期ワード送信
			delay_ms(1);
		}
	}
}

//1BYTE受信関数
static UBYTE getUartData(void)
{
	UBYTE rbuf;
	while(RCIF != 1);	//受信割込み待ち
	if(FERR == 1)		//フレーミングエラー処理
	{
		rbuf = RCREG;
		return '?';
	}
	else if(OERR == 1)	//オーバーランエラー処理
	{
		CREN = 0;
		CREN = 1;
		return '?';
	}
	return RCREG;
}

//1Byte送信関数
void sendChar(UBYTE c)
{
	while(TXIF != 1);
	TXREG = c;
}

//カメラコマンド送信関数
static void sendCom(const UBYTE *com)
{
	UBYTE i = 0;

	for(i = 0;i < 6;i++)
	{
		sendChar(com[i]);
	}
	while(!TRMT);	//111125 送信終了までwait
}

//データ取得用ACK送信
static void sendAckID(UBYTE *id)
{
	UBYTE i = 0;

	for(i = 0;i < 6;i++)
	{
		sendChar(id[i]);
	}
	while(!TRMT);
}

/* 1パケット分のデータsave */
static void saveCamPack(UDWORD * WAddr)
{
	UBYTE Ret;

	Ret = flash_Write_Page((UDWORD)*WAddr,(UWORD)64,(UBYTE *)Rbuf2.Data);
	*WAddr += (UDWORD)64;
	Ret = flash_Write_Page((UDWORD)*WAddr,(UWORD)64,(UBYTE *)Rbuf3.Data);
	*WAddr += (UDWORD)64;
}

/* 1パケット分のデータload */
static void loadCamPack(UDWORD * RAddr)
{
	UBYTE Ret;

	Ret = flash_Read_Data((UDWORD)*RAddr,64UL,(UBYTE *)Rbuf2.Data);
	*RAddr += 64UL;
	Ret = flash_Read_Data((UDWORD)*RAddr,64UL,(UBYTE *)Rbuf3.Data);
	*RAddr += 64UL;
}

/*-----------------------------------------------------------------------*/

//画像データ取得(直接変調用)
void getPicData(void)
{
	UBYTE idCount1 = 0;			//パッケージID(LSB)
	UBYTE idCount2 = 0;			//パッケージID(MSB)
	UWORD i = 0;

	//ACK0を送信バッファにロード
	for(i=0;i<6;i++)
	{
		send_buf[i] = ACK0[i];
	}
	i = 0;

	while(dlength > 122)
	{
		//IDカウント加算
		send_buf[4] = idCount1;
		send_buf[5] = idCount2;

		sendAckID(send_buf);		//ACK(データ取得ID)送信

		//画像データ受信
		for(i=0;i<64;i++)
		{
			Rbuf2.Data[i] = getUartData();
		}
		i = 0;
		for(i=0;i<64;i++)
		{
			Rbuf3.Data[i] = getUartData();
		}
		i = 0;

		dlength -= 122;

		//イメージデータIDの加算処理
		if(idCount1 == 0xFF)	//パッケージIDのLSBがFFなら桁上がり
		{
			idCount1 = 0;
			idCount2++;
		}
		else
		{
			idCount1++;
		}
	}

	/***最終パッケージの処理***/
	if(dlength != 0)
	{
		//IDカウント加算
		send_buf[4] = idCount1;
		send_buf[5] = idCount2;

		sendAckID(send_buf);		//ACK(データ取得ID)送信

		if(dlength > 58)
		{
			for(i=0;i<64;i++)
			{
				Rbuf2.Data[i] = getUartData();
			}
			i = 0;
			dlength = dlength - 60;
			for(i=0;i<(dlength+2);i++)
			{
				Rbuf3.Data[i] = getUartData();
			}
			for(i=(dlength+2);i<64;i++)
			{
				Rbuf3.Data[i] = 0xFF;
			}
		}
		else if(dlength == 58)
		{
			for(i=0;i<64;i++)
			{
				Rbuf2.Data[i] = getUartData();
			}
			i = 0;
			for(i=0;i<64;i++)
			{
				Rbuf3.Data[i] = 0xFF;
			}
		}
		else
		{
			for(i=0;i<(dlength+6);i++)
			{
				Rbuf2.Data[i] = getUartData();
			}
			for(i=(dlength+6);i<64;i++)
			{
				Rbuf2.Data[i] = 0xFF;
			}
			i = 0;
			for(i=0;i<64;i++)
			{
				Rbuf3.Data[i] = 0xFF;
			}
		}

		sendCom(AckEnd);
	}
	else
	{
		sendCom(AckEnd);
	}
}

//文字列送信関数
/*
void sendString(const UBYTE* p)
{
	while(*p)
		sendChar(*p++);
}
*/

static UBYTE mk_pn9(void)
{
	static UWORD	reg = 0x0021;
	UWORD			out,fb;

	out = reg & 0x0001;
	fb = (reg ^ (reg >> 4U)) & 0x0001;
	reg = (reg >> 1) | (fb << 8);

	return((UBYTE)out);
}

static void set_pn9(void){
	UBYTE i;

    send_buf[0] = 0x00;
	for(i = 0; i < 8; i++){
		send_buf[0] |= mk_pn9() << i;
	}

//	return(send_buf[0]);
}

/* PN9データ送信 */
void send_pn9(void)
{
//	UWORD i;
	UBYTE i;

	initbau(BAU_HIGH);								//UARTの初期化 115.2kbps
//	initbau(BAU_LOW);								//14.4kbps
//	initbau(0x5F);									//19.2kbps
//	initbau(0x1F);									//57.6kbps
	CREN = Bit_Low;									//受信禁止

//	Mod_SW = Bit_Low;						//変調ON
//	max2828_txon();
//	PA_SW  = Bit_High;

//	delay_ms(1);

	while(1){
//	for(i = 0; i < 10000; i++){
//        set_pn9();
//		sendChar(send_buf[0]);
		for(i=0; i<64; i++){
			send_buf[0] = PRBS9[i];
			sendChar(send_buf[0]);
		}
		i = 0;
	}
}

void send_55(void){
	initbau(BAU_HIGH);								//UARTの初期化 115.2kbps
	CREN = Bit_Low;									//受信禁止
	send_buf[0] = 0x55;
//	send_buf[0] = 0x00;
	while(1){
		sendChar(send_buf[0]);
	}
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

void change_downlink_baurate(UBYTE bau){
    BAU_WHEN_DOWNLINK = bau;
}
