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

static UBYTE send_buf[6];
static UBYTE rData[12];

const UBYTE STR[] = {"ABCDEFGH\r\n"};

void            initbau(UBYTE);
UBYTE           getUartData(UBYTE);
void            sendChar(UBYTE);
void            send_01(void);
void            send_AB(void);
void            send_OK(void);
void            send_NG(void);

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

void initbau(UBYTE bau)
{
    if((bau != BAU_LOW ) &&
       (bau != BAU_MIDDLE) &&
       (bau != BAU_HIGH))
    {
            bau = BAU_WITH_OBC;
    }
	BRGH    = Bit_High;
	BAUDCTL = 0x08;	
	BAULATE = bau;
	SPEN    = Bit_High;
}


void sendChar(UBYTE c)
{
	while(TXIF != 1);
	TXREG = c;
}

void send_01(void){
    UBYTE Preamblecount = 0;
    sendChar('\r');
    sendChar('\n');
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
    sendChar('\r');
    sendChar('\n');
    for(Preamblecount=0 ;Preamblecount <100; Preamblecount++){
        sendChar('A');
        __delay_us(20);
        sendChar('B');
        __delay_us(20);
    }
    sendChar('\r');
    sendChar('\n');
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
        clock_in_tst ++;
    }
}
//XXX
void send_dummy_data_timer(UBYTE time_command){
    CREN = Bit_Low;
    TXEN = Bit_High;
	UINT clock_in_tst = 0;
    delay_ms(1000);
    while(get_timer_counter_min() < time_command){
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
        set_timer_counter_only_getUart(0);
        while(RCIF != 1){
            if(get_timer_counter_only_getUart() > 1000) break;
        }
        return RCREG;
    }else{                  //C = CAM1 mode
        if(OERR || FERR){
            CREN = 0;
            CREN = 1;
        }
        while(RCIF != 1){
            if(CAM1 == 0) break;
        }
        return RCREG;
    }
}