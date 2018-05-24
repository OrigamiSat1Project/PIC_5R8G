#include <xc.h>
//#include "pic16f886.h"

#include "UART.h"
#include "FROM.h"
#include "typedefine.h"

void Receive_8split_JPEG(UDWORD Roop_adr, UDWORD Jump_adr){
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
            flash_Write_Data(FROM_Write_adr, (UDWORD)(i), &Buffer);
            /* Comment
             * ==================================================================== 
             * Jump to next group's first sector
             * ====================================================================
             */
            FROM_Write_adr = Roop_adr +(UINT)(receiveEndJpegFlag >> 4) * Jump_adr;
            receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag. Reset 0xFF flag in receiveEndJpegFlag
            receiveEndJpegFlag += 0x10;     //+1 8split_cnt in receiveEndJpegFlag.
            //After writing 8 sector, 8split_End =1 in receiveEndJpegFlag
            }
           else{
               receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag
           }
       }
        //FIXME for simulator
       flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Buffer);
       FROM_Write_adr += (UDWORD)(MaxOfMemory);
    }
    send_OK();
}

void Receive_thumbnail_JPEG(UDWORD Roop_adr){
//   /* Comment
//    * ===================================================================================================
//    * Erase sectors before writing FROM
//    * Original JPEG use 16 sectors and 1/4 JPEG use 8 sectors.
//    * We erase 16 sectors from Roop_adr in this Code.
//       ===================================================================================================
//    * Code
//    * ===================================================================================================
//    * Erase_sectors_before_Write(Roop_adr);
//    * ===================================================================================================
//    */
//    UBYTE Buffer[MaxOfMemory];
//    UBYTE receiveEndJpegFlag = 0x00;
//    UDWORD FROM_Write_adr = Roop_adr;         //Reset FROM_Write_adr
//    //FIXME for simulator
//    //sendChar('R');
//    
//    /*  How to use receiveEndJpegFlag
// * ===============================================================================================================
// *  Bit7            Bit6            Bit5        Bit4        Bit3        Bit2        Bit1                Bit0
// *  8split_End      8split_cnt2     cnt1        cnt0        ----        ----        Flag_0x0E           Flag_0xFF
// *  
// *  Initialize                 = 0x00  (0b00000000)
// *  Detect JPEG marker 0xFF    = 0x01
// *  End of receive JPEG        = 0x03  (0b00000011)
// *  During writing group 1     = 0x00  (0b00000000)
// *  During writing group 2     = 0x10  (0b00010000)
// *  During writing group 3     = 0x20  (0b00100000)
// *  During writing group 4     = 0x30  (0b00110000)
// *  During writing group 5     = 0x40  (0b01000000)
// *  During writing group 6     = 0x50  (0b01010000)
// *  During writing group 7     = 0x60  (0b01100000)
// *  During writing group 8     = 0x70  (0b01110000)
// *  After Writing              = 0x80  (0b10000000)
// * ===============================================================================================================
// */
//    
//    while((receiveEndJpegFlag  & 0x80) != 0x80){
//        for (UINT i = 0; i < MaxOfMemory; i++) {
//           while (RCIF != 1);
//           Buffer[i] = RCREG;
//           if((receiveEndJpegFlag & 0x01) == 0x00 && Buffer[i] == FooterOfJPEG[0]){
//               receiveEndJpegFlag |= 0x01;     //Flag_0xFF in receiveEndJPEGFlag = 1
//               //sendChar('1');
//           }
//           /* Comment
//            * ===================================================================================================
//            * Jump to next sector of FROM
//            * When 8split_end_flag in receiveEndJpegFlag is low, we should jumpt to next sector by change FROM_Writer_adr
//            * and +1count 8split_cnt in receiveEndJpegFlag.
//            * ===================================================================================================
//            */
//            else if ((receiveEndJpegFlag & 0x01) == 0x01 && Buffer[i] == FooterOfJPEG[1]){   //when change of FROM sector
//            //save data before jump to next sector
//            flash_Write_Data(FROM_Write_adr, (UDWORD)(i), &Buffer);
//            /* Comment
//             * ==================================================================== 
//             * Jump to next group's first sector
//             * ====================================================================
//             */
//            FROM_Write_adr = Roop_adr +(UINT)(receiveEndJpegFlag >> 4) * Jump_adr;
//            receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag. Reset 0xFF flag in receiveEndJpegFlag
//            receiveEndJpegFlag += 0x10;     //+1 8split_cnt in receiveEndJpegFlag.
//            //After writing 8 sector, 8split_End =1 in receiveEndJpegFlag
//            }
//           else{
//               receiveEndJpegFlag &= ~0x0f;    //Clear low order 4bit of receiveEndJpegFlag
//           }
//       }
//        //FIXME for simulator
//       flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Buffer);
//       FROM_Write_adr += (UDWORD)(MaxOfMemory);
//    }
//    send_OK();
}