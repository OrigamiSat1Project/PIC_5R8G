#include <xc.h>
//#include "pic16f886.h"

#include "UART.h"
#include "FROM.h"
#include "typedefine.h"

void ReceiveJPEG(UDWORD Roop_adr){
   /* Comment
    * ===================================================================================================
    * Erase sectors before writing FROM
    * Original JPEG use 16 sectors and 1/4 JPEG use 8 sectors.
    * We erase 16 sectors from Roop_adr in this Code.
       ===================================================================================================
    * Code
    * ===================================================================================================
    * Erase_sectors_before_Write(Roop_adr);
    * ===================================================================================================
    */
    UBYTE Buffer[MaxOfMemory];
    UBYTE receiveEndJpegFlag = 0x00;
    UDWORD FROM_Write_adr = Roop_adr;         //Reset FROM_Write_adr
    //FIXME for simulator
    //sendChar('R');
    
    while((receiveEndJpegFlag  & 0x80) != 0x80){
        for (UINT i = 0; i < MaxOfMemory; i++) {
           while (RCIF != 1);
           Buffer[i] = RCREG;
           if((receiveEndJpegFlag & 0x01) == 0x00 && Buffer[i] == FooterOfJPEG[0]){
               receiveEndJpegFlag |= 0x01;     //Flag_0xFF in receiveEndJPEGFlag = 1
               //sendChar('1');
           }
           /* Comment
            * ===================================================================================================
            * Jump to next sector of FROM
            * When 8split_end_flag in receiveEndJpegFlag is low, we should jumpt to next sector by change FROM_Writer_adr
            * and +1count 8split_cnt in receiveEndJpegFlag.
            * ===================================================================================================
            */
            else if ((receiveEndJpegFlag & 0x01) == 0x01 && Buffer[i] == FooterOfJPEG[1]){   //when change of FROM sector
            //save data before jump to next sector
            //FIXME for simulator
            //flash_Write_Data(FROM_Write_adr, (UDWORD)(i), &Buffer);
            //Jump to next Sector of FROM
            FROM_Write_adr &= 0xffff0000 ;        //Clear low order 2BYTE of FROM_Write_adr. Clear the memory address in previous sector
            //Jump to next part of sector ex.1¨3¨5¨7¨9¨B¨D¨F
            if((FROM_Write_adr >> 16) % 2 == 1){
                FROM_Write_adr += 0x20000;
            }else{
                FROM_Write_adr += 0x10000;
            }
            receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag. Reset 0xFF flag in receiveEndJpegFlag
            receiveEndJpegFlag += 0x10;     //+1 8split_cnt in receiveEndJpegFlag.
            //After writing 8 sector, 8split_End =1 in receiveEndJpegFlag
            }
           else{
               receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag
           }
       }
        //FIXME for simulator
       //flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Buffer);
       FROM_Write_adr += (UDWORD)(MaxOfMemory);
    }
    send_OK();
}