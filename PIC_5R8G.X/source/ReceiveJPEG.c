#include <xc.h>
//#include "pic16f886.h"

#include "stdio.h"
#include "string.h"
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"
#include "typedefine.h"

void ReceiveJPEG(UDWORD FROM_Write_adr){
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
    sendChar('R');
    while(receiveEndJpegFlag  & 0x80 != 0x80){
        for (UINT i = 0; i < MaxOfMemory; i++) {
           while (RCIF != 1);
           Buffer[i] = RCREG;
           if(receiveEndJpegFlag & 0x01 == 0x00 && RCREG == FooterOfJPEG[0]){
               receiveEndJpegFlag |= 0x01;     //Flag_0xFF in receiveEndJPEGFlag = 1
               //sendChar('1');
           }
           /* Comment
            * ===================================================================================================
            * Jump to next sector of FROM
            * When 8split_end_flag in receiveEndJpegFlag is low, we should jumpt to next sector by change FROM_Writer_adr
            * and +1count 8split_cnt in receiveEndJpegFlag.
               ===================================================================================================
            * Code
            * ===================================================================================================
            * else if (receiveEndJpegFlag & 0x01 == 0x01 && RCREG == FooterOfJPEG[1]){   //when change of FROM sector
            *  //save data before jump to next sector
            *  flash_Write_Data(FROM_Write_adr, (UDWORD)(i), &Buffer);
            *  //Jump to next Sector of FROM
            *  FROM_Write_adr &= ~0xffff;        //Clear low order 2BYTE of FROM_Write_adr. Clear the memory address in previous sector
            *  FROM_Write_adr += 0x10000;         //Jump to next sector
            *  receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag. Reset 0xFF flag in receiveEndJpegFlag
            *  receiveEndJpegFlag += 0x10;     //+1 8split_cnt in receiveEndJpegFlag.
            *  //After writing 8 sector, 8split_End =1 in receiveEndJpegFlag
            * }
            * ===================================================================================================
             */
           else{
               receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag
           }
       }
       flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Buffer);
       FROM_Write_adr += (UDWORD)(MaxOfMemory);
    }
    send_OK();
}