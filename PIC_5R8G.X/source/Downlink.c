#include <xc.h>
//#include "pic16f886.h"

#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"

void downlink(UBYTE);

void Downlink(UDWORD Roop_adr, UDWORD Jump_adr, UBYTE Identify_8split){
    /*  How to use Identify_8split
     * ========================================================================
     *  Bit7    Bit6    Bit5    Bit4    Bit3    Bit2    Bit1    Bit0
     *  8/8     7/8     6/8     5/8     4/8     3/8     2/8     1/8
     * ========================================================================
     */
    sendChar('P');
    while(CAM2 == 1);   //  wait 5V SW
//    if(CAMERA_POW == 1){
//        onAmp();
//    }
    UINT sendBufferCount = 0;
    const UINT JPGCOUNT = 5000;
    UBYTE Buffer[MaxOfMemory];
    UDWORD FROM_Read_adr = Roop_adr;
    UINT readFROM_Count = 0;                //How many sectors did you read in this while statement.
    
    UBYTE receiveEndJpegFlag = 0x00;
    /* How to use receiveEndJpegFlag
     * =========================================================================
     * Bit7    Bit6    Bit5    Bit4    Bit3    Bit2    Bit1    Bit0
     * ----    ----    ----    ----    ----    ----    0x0e    0x0f  
     * =========================================================================
     */
    CREN = Bit_Low;
    TXEN = Bit_High;
    /* Comment
     * =============================================================
     * Which groups do you read by reading Identify_8split.
     * Flag is ON : flash_Read_Data
     * Flag is OFF : FROM_Read_adr is jumping to next sector's start address.
     * =============================================================
     */
    //FIXME ; debug
    send_01();
    while(CAM2 == 0){
        if(readFROM_Count >= 8) readFROM_Count = 0;
        FROM_Read_adr = Roop_adr + readFROM_Count * Jump_adr;
        if(Identify_8split & (0x01<<readFROM_Count)){
            flash_Read_Data(FROM_Read_adr, (UDWORD)(MaxOfMemory), &Buffer);
            for(UINT i=0; i<MaxOfMemory; i++){
                downlink(Buffer[i]);
                if(receiveEndJpegFlag = 0x00 && Buffer[i] == FooterOfJPEG[0]){
                    receiveEndJpegFlag |= 0x01;
                }
                else if(receiveEndJpegFlag = 0x01 && Buffer[i] == FooterOfJPEG[1]){
                    receiveEndJpegFlag &= 0x00;
                    readFROM_Count ++;
                    //FIXME : debug
                    send_01();
                    sendChar((UBYTE)(FROM_Read_adr >> 16));
                    send_01();
                    break;
                }
                else{
                    receiveEndJpegFlag &= 0x00;
                }
            }
            FROM_Read_adr += (UDWORD)(MaxOfMemory);

//             //  for rest
//            if(sendBufferCount % JPGCOUNT == 0){
//                offAmp();
//                __delay_ms(3000);
//                if(CAMERA_POW == 1){
//                    onAmp();
//                }
//                send_01();  //  send preamble
//            }
            
            //  WDT dealing
            sendBufferCount ++;
            if (sendBufferCount % 20 == 0) {
                CLRWDT();
                WDT_CLK = ~WDT_CLK;
            }
        }
        else{
            readFROM_Count ++;
        }
    }
    offAmp();
    send_OK();
}

void downlink(UBYTE buf){
    sendChar(buf);
    //  TODO : to be optimisation
    //FIXME : debug by uart
    //__delay_us(20);
}