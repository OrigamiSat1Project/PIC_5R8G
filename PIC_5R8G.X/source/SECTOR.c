#include "stdio.h"
#include "string.h"
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"

void Erase_sectors(UBYTE Sector_start_byte, UBYTE Amount_of_erase_sector){
   /* Comment
    * ======================================================================================
    * Make Erase sector mode
    * We have to delete sectors in order to rewrite data in the sectors.
    * Bulk Erase takes long time (10s) so we should use sector erase.
    * Receive sector's identificate address and how many sectors want to delete, delete sectors from received sector.
    * ======================================================================================
    * Code
    * ======================================================================================
    *      UDWORD FROM_sector_adr = (UDWORD)Sector_start_byte<<16;    >   //We have to shift 16bit to move sector_start_address
    *      for (UBYTE i=0x00; i<Amount_of_erase_sector; i++){     > 
    *          flash_Erase(FROM_sector_adr, S_ERASE);
    *          FROM_sector_adr += 0x10000;                //Jump to next sector which you want to delete
    *          CLRWDT();
    *          WDT_CLK = ~WDT_CLK;
    *      }
    * ======================================================================================
    */
}