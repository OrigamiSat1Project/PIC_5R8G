#include <xc.h>
//#include "pic16f886.h"

#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"

void Downlink(UDWORD Roop_adr, UBYTE Identify_8split){
    sendChar('P');
    while(CAM2 == 1);   //  wait 5V SW
    UINT sendBufferCount = 0;
    const UINT JPGCOUNT = 5000;
    UBYTE Buffer[MaxOfMemory];
    UDWORD FROM_Read_adr = Roop_adr;
    UINT readFROM_Count = 0;                //How many sectors did you read in this while statement.
    /*  How to use Identify_8split
     * ===========================================================================================
     *  Bit7        Bit6        Bit5        Bit4        Bit3        Bit2        Bit1        Bit0
     *  8/8         7/8         6/8         5/8         4/8         3/8         2/8         1/8
     * ===========================================================================================
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
    while(CAM2 == 0){
        /*
        if(Identify_8split & (0x01<<readFROM_Count)){   >
            flash_Read_Data(FROM_Read_adr, (UDWORD)(MaxOfMemory), &Buffer);
        }
        else{
            FROM_Read_adr &= ~0xffff;            //Clear under lower 2bytes
            FROM_Read_adr += 0x10000;           //Jump to next sector's address
        }
        readFROM_Count +=1;
        */
        flash_Read_Data(FROM_Read_adr, (UDWORD)(MaxOfMemory), &Buffer);
        if(sendBufferCount % JPGCOUNT == 0){
            offAmp();
            __delay_ms(3000);
        if(CAMERA_POW == 1){
            onAmp();
            }
            send_01();  //  send preamble
        }
        for(UINT i=0;i<MaxOfMemory;i++){
            sendChar(Buffer[i]);
            //sendChar(0x00);
            __delay_us(20);
        }
        FROM_Read_adr += (UDWORD)(MaxOfMemory);
        sendBufferCount ++;
        if (sendBufferCount % 20 == 0) {
            CLRWDT();
            WDT_CLK = ~WDT_CLK;
        }
    }
    offAmp();
    send_OK();
}