#include <xc.h>
//#include "pic16f886.h"

#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"
#include "Timer.h"

void downlinkChar(UBYTE);
void downlinkRest(UBYTE);

void Downlink(UDWORD Roop_adr, UDWORD Jump_adr, UBYTE Identify_8split){
    /*  How to use Identify_8split
     * ========================================================================
     *  Bit7    Bit6    Bit5    Bit4    Bit3    Bit2    Bit1    Bit0
     *  8/8     7/8     6/8     5/8     4/8     3/8     2/8     1/8
     * ========================================================================
     */
    sendChar(0x50);
    while(CAM2 == 1);   //  wait 5V SW
    if(CAMERA_POW == 1){
        onAmp();
    }
    UINT sendBufferCount = 1;
    const UINT JPGCOUNT = 300;
    UBYTE Buffer[MaxOfMemory];
    UDWORD FROM_Read_adr = Roop_adr;
    UINT readFROM_Count = 0;                //How many sectors did you read in this while statement.

    UBYTE receiveEndJpegFlag = 0x00;
    CREN = Bit_Low;
    TXEN = Bit_High;
    /* Comment
     * =============================================================
     * Which groups do you read by reading Identify_8split.
     * Flag is ON : flash_Read_Data
     * Flag is OFF : FROM_Read_adr is jumping to next sector's start address.
     * =============================================================
     */
    send_01();
    //BUG
    //timer_counter = 0;
    /* Comment
     * =========================================================================
     * Significant change
     * We have used 0xff, 0x1e flag in this function.
     * We will use only 0xff flag
     *
     * How to use receiveEndJpegFlag
     * Bit7    Bit6    Bit5    Bit4    Bit3    Bit2    Bit1    Bit0
     * ----    ----    cnt5    cnt4    cnt3    cnt2    cnt1    cnt0_0xff
     * =========================================================================
     */
    while(CAM2 == 0){
        if(readFROM_Count >= 8){
            readFROM_Count = 0;
            FROM_Read_adr = Roop_adr;
            receiveEndJpegFlag = 0x00;
        }
        if((Identify_8split & (0x01<<readFROM_Count)) == (0x01<<readFROM_Count)){
            flash_Read_Data(FROM_Read_adr, (UDWORD)(MaxOfMemory), &Buffer);
            for(UINT i=0; i<MaxOfMemory; i++){
                downlinkChar(Buffer[i]);
                if(Buffer[i] == FROM_default_data){
                    receiveEndJpegFlag += 0x01;
                }else{
                    receiveEndJpegFlag &= 0x00;
                }
                if(receiveEndJpegFlag >= (UBYTE)(MaxOfMemory)*2){
                    receiveEndJpegFlag &= 0x00;
                    readFROM_Count ++;
                    FROM_Read_adr = Roop_adr + readFROM_Count * Jump_adr;
                    downlinkRest('1');
                    __delay_ms(3000);
                    break;
                }
            }
            FROM_Read_adr += (UDWORD)(MaxOfMemory);

            //  FIXME : TIMER2
             //  for rest
            if(timer_counter >= 10000){     //10s
                downlinkRest('A');
                sendBufferCount = 0;
            }

            //  WDT dealing
            if(timer_counter == 40){        //40ms
                CLRWDT();
                WDT_CLK = ~WDT_CLK;
            }
//            if(timer_counter == 20){
//                CLRWDT();
//                WDT_CLK = ~WDT_CLK;
//            }
        }
        else{
            readFROM_Count ++;
            FROM_Read_adr = Roop_adr + readFROM_Count * Jump_adr;
        }
    }
    offAmp();
    CREN = Bit_High;
    TXEN = Bit_Low;
}

void downlinkChar(UBYTE buf){
    sendChar(buf);
    __delay_us(20);
}

void downlinkRest(UBYTE c){
    if (c == '1'){
        send_01();
    }else{
        send_AB();
    }
    offAmp();
    __delay_ms(5000);
    if(CAMERA_POW == 1){
        onAmp();
    }
    if (c == '1'){
        send_01();
    }else{
        send_AB();
    }
    timer_counter = 0;
}
