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
    BUSY = 0;
    UDWORD FROM_sector_adr = (UDWORD)Sector_start_byte<<16;       //We have to shift 16bit to move sector_start_address
    for (UBYTE i=0x00; i<Amount_of_erase_sector; i++){
        flash_Erase(FROM_sector_adr, S_ERASE);
        FROM_sector_adr += 0x10000;                //Jump to next sector which you want to delete
        CLRWDT();
        WDT_CLK = ~WDT_CLK;
    }
    BUSY = 1;
}

void Erase_sectors_before_Write(UDWORD tmp_adr_erase, UDWORD Jump_adr){
   /* Comment
    * ==========================================================================
    * Erase sectors before writing FROM
    * JPEG will receive by 8split. Each group needs amount of Jump_adr sectors.
    * So we have to erase Jump_adr*8 sectors
    * ==========================================================================
    */
    BUSY = 0;
    UINT Amount_of_erase_sector = (UINT)(Jump_adr >> 16) * 8;
    for (UINT i=0; i<Amount_of_erase_sector; i++){
        flash_Erase(tmp_adr_erase,S_ERASE);
        tmp_adr_erase += 0x10000;         //Jump to next sector's start address
        CLRWDT();
        WDT_CLK =~WDT_CLK;
    }
    BUSY = 1;
}
