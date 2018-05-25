#include <xc.h>
//#include "pic16f886.h"

#include "UART.h"
#include "time.h"
#include "FROM.h"
#include "SECTOR.h"
#include "MAX2828.h"
#include "typedefine.h"

void Receive_8split_JPEG(UDWORD Roop_adr, UDWORD Jump_adr){
   /* Comment
    * ===================================================================================================
    * Erase sectors before writing FROM
    * Original JPEG use 16 sectors and 1/4 JPEG use 8 sectors.
    * We erase 16 sectors from Roop_adr in this Code.
       ===================================================================================================
    */
    Erase_sectors_before_Write(Roop_adr);
    
    
    UBYTE Buffer[MaxOfMemory];
    UBYTE receiveEndJpegFlag = 0x00;
    UDWORD FROM_Write_adr = Roop_adr;         //Reset FROM_Write_adr
    //FIXME for simulator
    //sendChar('R');
    
    /*  How to use receiveEndJpegFlag
 * ===============================================================================================================
 *  Bit7            Bit6            Bit5        Bit4        Bit3        Bit2        Bit1                Bit0
 *  8split_End      8split_cnt2     cnt1        cnt0        ----        ----        Flag_0x0E           Flag_0xFF
 *  
 *  Initialize                 = 0x00  (0b00000000)
 *  Detect JPEG marker 0xFF    = 0x01
 *  End of receive JPEG        = 0x03  (0b00000011)
 *  During writing group 1     = 0x00  (0b00000000)
 *  During writing group 2     = 0x10  (0b00010000)
 *  During writing group 3     = 0x20  (0b00100000)
 *  During writing group 4     = 0x30  (0b00110000)
 *  During writing group 5     = 0x40  (0b01000000)
 *  During writing group 6     = 0x50  (0b01010000)
 *  During writing group 7     = 0x60  (0b01100000)
 *  During writing group 8     = 0x70  (0b01110000)
 *  After Writing              = 0x80  (0b10000000)
 * ===============================================================================================================
 */
    UINT index_of_Buffer = 0;
    while((receiveEndJpegFlag  & 0x80) != 0x80){
        while (RCIF != 1);
        Buffer[index_of_Buffer] = RCREG;
        if((receiveEndJpegFlag & 0x01) == 0x00 && Buffer[index_of_Buffer] == FooterOfJPEG[0]){
            receiveEndJpegFlag |= 0x01;
            index_of_Buffer++;
        }
        else if ((receiveEndJpegFlag & 0x01) == 0x01 && Buffer[index_of_Buffer] == FooterOfJPEG[1])
        {
            flash_Write_Data(FROM_Write_adr, (UDWORD)(index_of_Buffer + 1), &Buffer);
            index_of_Buffer = 0;
            
            //  Jump to next group's first sector & change flag
            receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag. Reset 0xFF flag in receiveEndJpegFlag
            receiveEndJpegFlag += 0x10;     //+1 8split_cnt in receiveEndJpegFlag.
            FROM_Write_adr = Roop_adr +(UINT)(receiveEndJpegFlag >> 4) * Jump_adr;
            // FIXME : for dubug
            offAmp();
            TXEN = Bit_High;
            sendChar(receiveEndJpegFlag);
            sendChar((UBYTE)(FROM_Write_adr >> 16));
        }
        else
        {
            receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag
            index_of_Buffer++;
        }
        if(index_of_Buffer == MaxOfMemory){
            flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Buffer);
            FROM_Write_adr += (UDWORD)(MaxOfMemory);
            index_of_Buffer = 0;
        }
    }
    //  FIXME : for debug
    send_OK();
    //  FIXME : for debug
    delay_ms(3000);
    offAmp();
    while(CAM2 == 1);   //  wait 5V SW
    TXEN = Bit_High;
    UDWORD FROM_Read_adr = Roop_adr;
    while(FROM_Read_adr <= FROM_Write_adr){
        flash_Read_Data(FROM_Read_adr, (UDWORD)(MaxOfMemory), &Buffer);
        for(UINT i=0;i<MaxOfMemory;i++){
            sendChar(Buffer[i]);
        }
        FROM_Read_adr += (UDWORD)(MaxOfMemory);
    }
}

void Receive_thumbnail_JPEG(UDWORD Roop_adr){
   /* Comment
    * ===================================================================================================
    * Erase sectors before writing FROM
    * Original JPEG use 16 sectors and 1/4 JPEG use 8 sectors.
    * We erase 16 sectors from Roop_adr in this Code.
       ===================================================================================================
    */
    //FIXME : simulator
    //Erase_sectors_before_Write(Roop_adr);
    
    
    UBYTE Buffer[MaxOfMemory];
    UBYTE receiveEndJpegFlag = 0x00;
    UDWORD FROM_Write_adr = Roop_adr;         //Reset FROM_Write_adr
    //FIXME for simulator
    //sendChar('R');
    
   /*  How to use receiveEndJpegFlag
    * ===============================================================================================================
    *  Bit7      Bit6     Bit5    Bit4        Bit3        Bit2        Bit1                Bit0
    *  ----      ----     ----    ----        ----        ----        Flag_0x0E           Flag_0xFF
    * ===============================================================================================================
    */
    UINT index_of_Buffer = 0;
    while((receiveEndJpegFlag  & 0x10) != 0x10){
        while (RCIF != 1);
        Buffer[index_of_Buffer] = RCREG;
        if((receiveEndJpegFlag & 0x01) == 0x00 && Buffer[index_of_Buffer] == FooterOfJPEG[0]){
            receiveEndJpegFlag |= 0x01;
            index_of_Buffer++;
        }
        else if ((receiveEndJpegFlag & 0x01) == 0x01 && Buffer[index_of_Buffer] == FooterOfJPEG[1])
        {
            //FIXME : simulator
            //flash_Write_Data(FROM_Write_adr, (UDWORD)(index_of_Buffer + 1), &Buffer);
            receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag. Reset 0xFF flag in receiveEndJpegFlag
            receiveEndJpegFlag += 0x10;     //+1 8split_cnt in receiveEndJpegFlag.
            index_of_Buffer = 0;
            //FIXME : simulator
            //sendChar(receiveEndJpegFlag);
            //sendChar((UBYTE)(FROM_Write_adr >> 16));
        }
        else
        {
            receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag
            index_of_Buffer++;
        }
        if(index_of_Buffer == MaxOfMemory){
            //FIXME : simulator
            //flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Buffer);
            FROM_Write_adr += (UDWORD)(MaxOfMemory);
            index_of_Buffer = 0;
        }
    }
    //  FIXME : for debug
    send_OK();
    //  FIXME : for debug
    delay_ms(3000);
    offAmp();
    while(CAM2 == 1);   //  wait 5V SW
    TXEN = Bit_High;
    UDWORD FROM_Read_adr = Roop_adr;
    while(FROM_Read_adr <= FROM_Write_adr){
        flash_Read_Data(FROM_Read_adr, (UDWORD)(MaxOfMemory), &Buffer);
        for(UINT i=0;i<MaxOfMemory;i++){
            sendChar(Buffer[i]);
        }
        FROM_Read_adr += (UDWORD)(MaxOfMemory);
    }
}
