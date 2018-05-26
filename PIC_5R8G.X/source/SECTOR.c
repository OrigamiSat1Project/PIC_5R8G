#include <xc.h>
#include "InitMPU.h"
#include "FROM.h"
#include "UART.h"

void Erase_sectors(UBYTE Sector_start_byte, UBYTE Amount_of_erase_sector){
   /* Comment
    * ======================================================================================
    * Make Erase sector mode
    * We have to delete sectors in order to rewrite data in the sectors.
    * Bulk Erase takes long time (10s) so we should use sector erase.
    * Receive sector's identificate address and how many sectors want to delete, delete sectors from received sector.
    * ======================================================================================
    */
    UDWORD FROM_sector_adr = (UDWORD)Sector_start_byte<<16;       //We have to shift 16bit to move sector_start_address
    for (UBYTE i=0x00; i<Amount_of_erase_sector; i++){      
        flash_Erase(FROM_sector_adr, S_ERASE);
        FROM_sector_adr += 0x10000;                //Jump to next sector which you want to delete
        CLRWDT();
        WDT_CLK = ~WDT_CLK;
    }
}

void Erase_sectors_before_Write(UDWORD tmp_adr_erase){
   /* Comment
    * ===================================================================================================
    * Erase sectors before writing FROM
    * Original JPEG use 16 sectors and 1/4 JPEG use 8 sectors.
    * We erase 16 sectors from Roop_adr in this Code.
    */
    //XXX BUSY signal during erase sectors
    BUSY = 0;
    //  FIXME : this is hardcoding. 
    const UINT Amount_of_erase_sector = 16;
    for (UINT i=0; i<Amount_of_erase_sector; i++){
        flash_Erase(tmp_adr_erase,S_ERASE);
        tmp_adr_erase += 0x10000;         //Jump to next sector's start address
        CLRWDT();
        WDT_CLK =~WDT_CLK;
    }
    BUSY = 1;
}