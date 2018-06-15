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
#include "Timer.h"


//?øΩœêÔøΩ?øΩÃêÈåæ
static UBYTE send_buf[6];	//?øΩ?øΩ?øΩM?øΩp?øΩo?øΩb?øΩt?øΩ@
static UBYTE rData[12];		//?øΩ?øΩ?øΩM?øΩp?øΩo?øΩb?øΩt?øΩ@
static USLONG dlength;		//?øΩf?øΩ[?øΩ^?øΩ?øΩ

extern UDWORD               g_data_adr;	//FROM?øΩ?øΩ?øΩÃÉA?øΩh?øΩ?øΩ?øΩX(?øΩf?øΩ[?øΩ^?øΩp)

//static bank2 volatile CamDataBuf	Rbuf2;	//?øΩÊëúÔøΩf?øΩ[?øΩ^?øΩp?øΩo?øΩb?øΩt?øΩ@
//static bank3 volatile CamDataBuf	Rbuf3;	//?øΩÊëúÔøΩf?øΩ[?øΩ^?øΩp?øΩo?øΩb?øΩt?øΩ@


const UBYTE STR[] = {"ABCDEFGH\r\n"};

//?øΩ÷êÔøΩ?øΩÃêÈåæ
//static void		initbau(void);
void            initbau(UBYTE);
UBYTE           getUartData(UBYTE);
void            sendChar(UBYTE);
void            send_01(void);
void            send_AB(void);
void            send_OK(void);
void            send_NG(void);

//  TODO : this should be tested by simulator & debug
static UBYTE BAU_WHEN_DOWNLINK = BAU_WITH_OBC;

//  Switch on power of amplifier
void onAmp(){
    initbau(BAU_WHEN_DOWNLINK);
    __delay_ms(10);
    CAMERA_POW = 0;
    CAMERA_SEL = 0;
    max2828_txon();
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

////?øΩÊëúÔøΩf?øΩ[?øΩ^?øΩ€ëÔøΩ
//void savePicData(void)
//{
//	UBYTE idCount1 = 0;			//?øΩp?øΩb?øΩP?øΩ[?øΩWID(LSB)
//	UBYTE idCount2 = 0;			//?øΩp?øΩb?øΩP?øΩ[?øΩWID(MSB)
//	UWORD i = 0;
////	UBYTE Ret;
//	UDWORD w_adr;
//
//	w_adr = g_data_adr;
//	//ACK0?øΩ?ëóêM?øΩo?øΩb?øΩt?øΩ@?øΩ…?øΩ?øΩ[?øΩh
//	for(i=0;i<6;i++)
//	{
//		send_buf[i] = ACK0[i];
//	}
//
//	while(dlength > 122)
//	{
//		//ID?øΩJ?øΩE?øΩ?øΩ?øΩg?øΩ?øΩ?øΩZ
//		send_buf[4] = idCount1;
//		send_buf[5] = idCount2;
//
//		sendAckID(send_buf);		//ACK(?øΩf?øΩ[?øΩ^?øΩÊìæID)?øΩ?øΩ?øΩM
//
//		//?øΩÊëúÔøΩf?øΩ[?øΩ^?øΩ?øΩ?øΩM
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
//		//?øΩC?øΩ?øΩ?øΩ[?øΩW?øΩf?øΩ[?øΩ^ID?øΩÃâÔøΩ?øΩZ?øΩ?øΩ?øΩ?øΩ
//		if(idCount1 == 0xFF)	//?øΩp?øΩb?øΩP?øΩ[?øΩWID?øΩ?øΩLSB?øΩ?øΩFF?øΩ»ÇÁåÖ?øΩ„Ç™?øΩ?øΩ
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
//	/***?øΩ≈èI?øΩp?øΩb?øΩP?øΩ[?øΩW?øΩÃèÔøΩ?øΩ?øΩ***/
//	if(dlength != 0)
//	{
//		//ID?øΩJ?øΩE?øΩ?øΩ?øΩg?øΩ?øΩ?øΩZ
//		send_buf[4] = idCount1;
//		send_buf[5] = idCount2;
//
//		sendAckID(send_buf);		//ACK(?øΩf?øΩ[?øΩ^?øΩÊìæID)?øΩ?øΩ?øΩM
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

//UART?øΩÃèÔøΩ?øΩ?øΩ?øΩ?øΩ
void initbau(UBYTE bau)
{
    if((bau != BAU_LOW ) &&
       (bau != BAU_MIDDLE) &&
       (bau != BAU_HIGH))
    {
            bau = BAU_WITH_OBC;
    }
	/*?øΩ{?øΩ[?øΩ?øΩ?øΩ[?øΩg?øΩ›íÔøΩ*/
	BRGH    = Bit_High;		//?øΩ{?øΩ[?øΩ?øΩ?øΩ[?øΩg?øΩ?øΩ?øΩ?øΩ?øΩ?øΩ?øΩ[?øΩh
	BAUDCTL = 0x08;			//16?øΩ{?øΩ?øΩ
	BAULATE = bau;
	SPEN    = Bit_High;		//?øΩV?øΩ?øΩ?øΩA?øΩ?øΩ?øΩ|?øΩ[?øΩg?øΩ›íÔøΩ
}

//1Byte?øΩ?øΩ?øΩM?øΩ÷êÔøΩ
void sendChar(UBYTE c)
{
	while(TXIF != 1);
	TXREG = c;
}

void send_tst_str(void){
    CREN = Bit_Low;
    TXEN = Bit_High;
	UINT clock_in_tst = 0;
//	initbau(BAU_HIGH);								//UART?øΩÃèÔøΩ?øΩ?øΩ?øΩ?øΩ 115.2kbps
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

void send_AB(void){
    UBYTE Preamblecount = 0;
    for(Preamblecount=0 ;Preamblecount <100; Preamblecount++){
        sendChar('A');
        __delay_us(20);
        sendChar('B');
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

void send_CRLF(void){
    sendChar('\r');
    sendChar('\n');
}

void echo_back(void){
    UBYTE testbuf1;
    UBYTE testbuf2;
    UDWORD echo_adr = g_data_adr;
    CREN = Bit_High;
    TXEN = Bit_High;
    __delay_ms(1000);
    //flash_Erase(g_data_adr,S_ERASE);    //g_data_adr?øΩ?øΩsector65536byte?øΩ?øΩ?øΩ?èú
    while(1){
        testbuf1 = getUartData(0x00);
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

UBYTE getDownlinkBAU(void){
    return BAU_WHEN_DOWNLINK;
}

UBYTE getUartData(UBYTE mode){
    if(mode == 'T'){        //T = Timer processing
        if(OERR || FERR){
            CREN = 0;
            CREN = 1;
        }
        while(RCIF != 1){
            if(timer_counter > 100) break;
        }
        return RCREG;
    }else if(mode == 'C'){  //C = CAM1 mode
        if(OERR || FERR){
            CREN = 0;
            CREN = 1;
        }
        while(RCIF != 1){
            if(CAM1 == 0) break;
        }
        return RCREG;
    }else{
        if(OERR || FERR){
            CREN = 0;
            CREN = 1;
        }
        while(RCIF != 1){
            if(CAM1 == 1)break;
        }
        return RCREG;
    }
}