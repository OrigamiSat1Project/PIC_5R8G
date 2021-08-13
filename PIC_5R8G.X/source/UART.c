#include <xc.h>
//#include "pic16f886.h"
//#include <stdlib.h>

//#include "typedefine.h"
#include "UART.h"
#include "Downlink.h"
#include "time.h"
#include "FROM.h"
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

// #define UART_DEBUG_MODE

//  Switch on power of amplifier
void onAmp(){
    initbau(BAU_WHEN_DOWNLINK);
    __delay_ms(10);
#ifdef UART_DEBUG_MODE
    CAMERA_POW = 1;
    CAMERA_SEL = 1;
#else
    CAMERA_POW = 0;
    CAMERA_SEL = 0;
    max2828_txon();
#endif
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
    UINT index_of_STR = 0;
    set_timer_counter_only_rest(0);
    while(CAM2 == 0){
        UBYTE buf = STR[index_of_STR];
        downlinkChar(buf);
        index_of_STR++;
        if(index_of_STR > 10){
            index_of_STR = 0;
        }
        if(get_timer_counter_only_rest() > get_downlink_time()){
            downlinkRest('1');
        }
    }
    CREN = Bit_High;
    TXEN = Bit_Low;
}
//XXX
void send_dummy_data_timer(UBYTE time_command){
    CREN = Bit_Low;
    TXEN = Bit_High;
    UINT index_of_STR = 0;
    
    while(get_timer_counter_min() < time_command){
        UBYTE buf = STR[index_of_STR];
        downlinkChar(buf);
        index_of_STR++;
        if(index_of_STR > 10){
            index_of_STR = 0;
        }
        if(get_timer_counter_only_rest() > get_downlink_time()){
            downlinkRest('1');
        }
    }
    CREN = Bit_High;
    TXEN = Bit_Low;
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