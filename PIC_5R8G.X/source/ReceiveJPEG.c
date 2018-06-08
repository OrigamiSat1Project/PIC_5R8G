#include <xc.h>
//#include "pic16f886.h"

#include "UART.h"
#include "time.h"
#include "FROM.h"
#include "SECTOR.h"
#include "MAX2828.h"
#include "typedefine.h"
#include "UART.h"

void Receive_8split_JPEG(UDWORD Roop_adr, UDWORD Jump_adr){
   /* Comment
    * ==========================================================================
    * Erase sectors before writing FROM
    * We erase Roop_adr + Jump_adr * 8 sectors before writing
    * ==========================================================================
    */
    Erase_sectors_before_Write(Roop_adr, Jump_adr);
    UBYTE Buffer[MaxOfMemory];
    UBYTE receiveEndJpegFlag = 0x00;
    UDWORD FROM_Write_adr = Roop_adr;         //Reset FROM_Write_adr

   /*  How to use receiveEndJpegFlag
    * ===============================================================================================================
    *  Bit7            Bit6            Bit5        Bit4        Bit3        Bit2        Bit1                Bit0
    *  8split_End      8split_cnt2     cnt1        cnt0        ----        ----        Flag_0x1E           Flag_0xFF
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
    * ===============================================================================================================
    */
    UINT index_of_Buffer = 0;
    //FIXME : debug
    sendChar(0x88);
    CREN = Bit_High;    //It is needed for integration with OBC
    //TXEN = Bit_High;
    while((receiveEndJpegFlag  & 0x80) != 0x80){
        Buffer[index_of_Buffer] = getUartData(0x00);
        if((receiveEndJpegFlag & 0x01) == 0x00 && Buffer[index_of_Buffer] == FooterOfJPEG[0]){
            receiveEndJpegFlag |= 0x01;
            index_of_Buffer++;
        }
        else if ((receiveEndJpegFlag & 0x01) == 0x01 && Buffer[index_of_Buffer] == FooterOfJPEG[1])
        {
            flash_Write_Data(FROM_Write_adr, (UDWORD)(index_of_Buffer + 1), &Buffer);
            index_of_Buffer = 0;
            //  Jump to next group's first sector & change flag
            receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag. Reset 0xFF flag
            receiveEndJpegFlag += 0x10;     //+1 8split_cnt in receiveEndJpegFlag.
            FROM_Write_adr = Roop_adr +(UINT)(receiveEndJpegFlag >> 4) * Jump_adr;
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
}

void Receive_thumbnail_JPEG(UDWORD Roop_adr, UDWORD Jump_adr){
   /* Comment
    * ==========================================================================
    * Erase sectors before writing FROM
    * We erase Roop_adr + Jump_adr * 8 sectors before writing
    * ==========================================================================
    */
    Erase_sectors_before_Write(Roop_adr, Jump_adr);
    UBYTE Buffer[MaxOfMemory];
    UBYTE receiveEndJpegFlag = 0x00;
    UDWORD FROM_Write_adr = Roop_adr;         //Reset FROM_Write_adr

   /*  How to use receiveEndJpegFlag
    * ====================================================================================================
    *  Bit7      Bit6     Bit5    Bit4               Bit3        Bit2        Bit1                Bit0
    *  ----      ----     ----    finish_cnt        ----        ----        Flag_0x0E           Flag_0xFF
    * ====================================================================================================
    */
    //FIXME : debug
    sendChar(0xbb);
    CREN = Bit_High;
    TXEN = Bit_High;
    UINT index_of_Buffer = 0;
    while((receiveEndJpegFlag  & 0x10) != 0x10)
    {
        Buffer[index_of_Buffer] = getUartData(0x00);
        if((receiveEndJpegFlag & 0x01) == 0x00 && Buffer[index_of_Buffer] == FooterOfJPEG[0]){
            receiveEndJpegFlag |= 0x01;
            index_of_Buffer++;
        }
        else if ((receiveEndJpegFlag & 0x01) == 0x01 && Buffer[index_of_Buffer] == FooterOfJPEG[1])
        {
            flash_Write_Data(FROM_Write_adr, (UDWORD)(index_of_Buffer + 1), &Buffer);
            index_of_Buffer = 0;
            receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag. Reset 0xFF flag
            receiveEndJpegFlag += 0x10;     //+1 8split_cnt in receiveEndJpegFlag.
            //FIXME : debug
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
}

void Receive_8split_H264(UDWORD Roop_adr, UDWORD Jump_adr){
   /* Comment
    * ==========================================================================
    * Erase sectors before writing FROM
    * We erase Roop_adr + Jump_adr * 8 sectors before writing
    * ==========================================================================
    */
    Erase_sectors_before_Write(Roop_adr, Jump_adr);
    UBYTE Buffer[MaxOfMemory];
    UBYTE receiveEndH264Flag = 0x00;
    UDWORD FROM_Write_adr = Roop_adr;         //Reset FROM_Write_adr

    /*  How to use receiveEndH264Flag
     * ===============================================================================================================
     *  Bit7            Bit6            Bit5        Bit4        Bit3                Bit2            Bit1                                Bit0
     *  8split_End      8split_cnt2     cnt1        cnt0        Flag_0x1e        Flag_0x01        Flag_0x00(Second time)           Flag_0x00(First time)
     *
     *  Initialize                 = 0x00  (0b00000000)
     *  Receive 0x00 (First)       = 0x01  (0b00000001)
     *  Receive 0x00 (Second)      = 0x03  (0b00000011)
     *  Receive 0x01               = 0x07  (0b00000111)
     *  During writing group 1     = 0x00  (0b00000000)
     *  During writing group 2     = 0x10  (0b00010000)
     *  During writing group 3     = 0x20  (0b00100000)
     *  During writing group 4     = 0x30  (0b00110000)
     *  During writing group 5     = 0x40  (0b01000000)
     *  During writing group 6     = 0x50  (0b01010000)
     *  During writing group 7     = 0x60  (0b01100000)
     *  During writing group 8     = 0x70  (0b01110000)
     * ===============================================================================================================
     */
    UINT index_of_Buffer = 0;
    //FIXME : debug
    sendChar(0x77);
    CREN = Bit_High;    //It is needed for integration with OBC
    //TXEN = Bit_High;
    while((receiveEndH264Flag  & 0x80) != 0x80){
        Buffer[index_of_Buffer] = getUartData(0x00);
        if((receiveEndH264Flag & 0x01) == 0x00 && Buffer[index_of_Buffer] == FooterOfH264[0]){
            receiveEndH264Flag |= 0x01;
            index_of_Buffer++;
        }
        else if((receiveEndH264Flag & 0x01) == 0x01 && Buffer[index_of_Buffer] == FooterOfH264[1]){
            receiveEndH264Flag |= 0x02;
            index_of_Buffer++;
        }
        else if((receiveEndH264Flag & 0x02) == 0x02 && Buffer[index_of_Buffer] == FooterOfH264[2]){
            receiveEndH264Flag |= 0x04;
            index_of_Buffer++;
        }
        else if ((receiveEndH264Flag & 0x04) == 0x04 && Buffer[index_of_Buffer] == FooterOfH264[3])
        {
            flash_Write_Data(FROM_Write_adr, (UDWORD)(index_of_Buffer + 1), &Buffer);
            index_of_Buffer = 0;

            //  Jump to next group's first sector & change flag
            receiveEndH264Flag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag.
            receiveEndH264Flag += 0x10;     //+1 8split_cnt in receiveEndH264Flag.
            FROM_Write_adr = Roop_adr +(UINT)(receiveEndH264Flag >> 4) * Jump_adr;
            sendChar(receiveEndH264Flag);
            sendChar((UBYTE)(FROM_Write_adr >> 16));
        }
        else
        {
            receiveEndH264Flag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag
            index_of_Buffer++;
        }

        if(index_of_Buffer == MaxOfMemory){
            flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Buffer);
            FROM_Write_adr += (UDWORD)(MaxOfMemory);
            index_of_Buffer = 0;
        }
    }
}

void Receive_ECC(UDWORD Roop_adr, UDWORD Jump_adr, UINT ECC_length){
   /* Comment
    * ==========================================================================
    * Erase sectors before writing FROM
    * We erase Roop_adr + Jump_adr * 8 sectors before writing
    * ==========================================================================
    */
    Erase_sectors_before_Write(Roop_adr, Jump_adr);
    UBYTE Buffer[MaxOfMemory];
    UBYTE receiveEndECCFlag = 0x00;
    UDWORD FROM_Write_adr = Roop_adr;         //Reset FROM_Write_adr

    /* Comment
     * =========================================================================
     * We calc the length of ECC from Command_length
     * =========================================================================
     */
    
   /*  How to use receiveEndECCFlag
    * ===============================================================================================================
    * Bit7    Bit6    Bit5    Bit4    Bit3    Bit2    Bit1    Bit0
    *                                 cnt3    cnt2    cnt1    cnt0
    *
    *  Initialize                 = 0x00  (0b00000000)
    * ===============================================================================================================
    */
    UINT index_of_Buffer = 0;
    UINT ECC_count = 0;
    //FIXME :
    sendChar(0xee);
    CREN = Bit_High;    //It is needed for integration with OBC
    //TXEN = Bit_High;
    while(receiveEndECCFlag != 0x08){
        while (RCIF != 1){
            if(CAM1 == 1) break;
        }
        Buffer[index_of_Buffer] = getUartData(0x00);
        if (receiveEndECCFlag < 0x07 && ECC_count == ECC_length/8)  
        {
            flash_Write_Data(FROM_Write_adr, (UDWORD)(index_of_Buffer + 1), &Buffer);
            index_of_Buffer = 0;
            //  Jump to next group's first sector & change flag
            receiveEndECCFlag += 0x01;     //+1 8split_cnt in receiveEndECCFlag.
            FROM_Write_adr = Roop_adr +(UINT)(receiveEndECCFlag) * Jump_adr;
            ECC_count = 0;
            //FIXME : debug
            sendChar(receiveEndECCFlag);
            sendChar((UBYTE)(FROM_Write_adr >> 16));
        }
        else if(receiveEndECCFlag = 0x07 && ECC_count == (ECC_length - (ECC_length/8) * 7))
        {
            flash_Write_Data(FROM_Write_adr, (UDWORD)(index_of_Buffer + 1), &Buffer);
            index_of_Buffer = 0;
            ECC_count = 0;
            receiveEndECCFlag += 0x01;
        }
        else
        {
            ECC_count++;
            index_of_Buffer++;
        }
        if(index_of_Buffer == MaxOfMemory){
            flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Buffer);
            FROM_Write_adr += (UDWORD)(MaxOfMemory);
            index_of_Buffer = 0;
        }
    }
}
