#include <xc.h>
//#include "pic16f886.h"

#include "stdio.h"
#include "string.h"
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"

// CONFIG1
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
//#pragma config WDTE = ON       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)
// CONFIG2
#pragma config BOR4V = BOR21V   // Brown-out Reset Selection bit (Brown-out Reset set to 2.1V)
#pragma config WRT = HALF       // Flash Program Memory Self Write Enable bits (0000h to 0FFFh write protected, 1000h to 1FFFh may be modified by EECON control)

UDWORD          g_data_adr  = (UDWORD)0x00000000;


#define JPGCOUNT 5000
#define MaxOfMemory 40  //  TODO : Use Bank function then magnify buffer size
const UBYTE FooterOfJPEG[] =  {0xff, 0xd9, 0x0e}; 

/*  How to use receiveEndJpegFlag
 *  Bit7            Bit6            Bit5        Bit4        Bit3        Bit2        Bit1                Bit0
 *  8split_End      8split_cnt2     cnt1        cnt0        ----        ----        Flag_0x0E           Flag_0xFF
 *  
 *  Initialize                  = 0x00
 *  Detect JPEG marker 0xFF     = 0x01
 *  End of receive JPEG         = 0x03  (0b00000011)
 *  During writing sector 2     = 0x10  (0b00010000)
 *  During writing sector 8     = 0x70  (0b01110000)
 *  After 8split_end            = 0x80  (0b10000000)
 */

void main(void){
    UDWORD			g1_data_adr = (UDWORD)0x00010000;
    //UDWORD			g2_data_adr = (UDWORD)0x00020000;   No need

    UBYTE Buffer[MaxOfMemory];
    UINT indexOfBuffer = 0;

    UDWORD FROM_Write_adr = g1_data_adr;
    UDWORD FROM_Read_adr  = g1_data_adr;
    UDWORD FROM_sector_adr = g1_data_adr;       //Each sector's first address kind of 0x00ÅõÅõ0000. Use in 'C' and 'D' command
    UDWORD Roop_adr = g1_data_adr;
    UDWORD FROM_Jump_next_sector = 0x10000;
    UINT roopcount = 0;

    init_mpu();
    //initbau(BAU_HIGH);                //115200bps
    //initbau(BAU_MIDDLE);              //57600bps
    initbau(BAU_LOW);                   //14400bps
    MAX2828_EN = 1;                     //MAX2828 ON
    __delay_us(100);                    //100us wait
    FLASH_SPI_EI();                     //enable SPI
    init_max2828();                     //init MAX2828
    Mod_SW = 0;                         //FSK modulation ON

    while(1){
        if(CAMERA_POW == 0){
            offAmp();
        }
        CREN = Bit_High;
        TXEN = Bit_High;
        UBYTE Command;
        
        while(RCIF != 1);
        Command = RCREG;
        while(Command != '5');
        
        while(RCIF != 1);
        Command = RCREG;
        //  TODO : Add time restrict of picture downlink (10s downlink, 5s pause)
        if(Command == 'P')
        {
            sendChar('P');
            while(CAM2 == 1);   //  wait 5V SW
            FROM_Read_adr = Roop_adr;
            UINT sendBufferCount = 0;
            CREN = Bit_Low;
            TXEN = Bit_High;
            while(FROM_Read_adr < FROM_Write_adr && CAM2 == 0){
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
        else if (Command == 'D')
        {
            while(CAM2 == 1);   //  wait 5V SW
            while(CAM2 == 0){
                if(CAMERA_POW == 1){
                    onAmp();
                }
                send_dummy_data();
            }
            offAmp();
            send_OK();
        }
        else if (Command == 'R')
        {
            /* Comment
             * ===================================================================================================
             * Erase sectors before writing FROM
             * Original JPEG use 16 sectors and 1/4 JPEG use 8 sectors.
             * We erase 16 sectors from Roop_adr in this Code.
                ===================================================================================================
             * Code
             * ===================================================================================================
             * const UINT Amount_of_erase_sector = 16;
             * UDWORD tmp_adr_erase = Roop_adr;     //Use only this for statement
             * for (i=0; i<Amount_of_erase_sector; i++){    >
             *      flash_Erase(tmp_adr_erase,S_ERASE);
             *      tmp_adr_erase += 0x10000;         //Jump to next sector's start address
             *      //
             *      if (i % 2 ==1){
             *          CLRWDT();
             *          WDT_CLK =~WDT_CLK;
             *      }
             * }
             * ===================================================================================================
              */            
            FROM_Write_adr = Roop_adr;         //Reset FROM_Write_adr
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

        /* Comment
         * ======================================================================================
         * Make Delete sector mode
         * We have to delete sectors in order to rewrite data in the sectors.
         * Bulk Erase takes long time (10s) so we should use sector erase.
         * Receive sector's identificate address and how many sectors want to delete, delete sectors from received sector.
         * ======================================================================================
         * Code
         * ======================================================================================
         * else if(Command = 'D'){
         *      UBYTE Amount_of_erase_sector_OBC;    //How many sectors do you want to delete
         *      while (RCIF != 1);
         *      FROM_sector_adr = (UDWORD)RCREG;
         *      FROM_sector_adr = FROM_sector_adr<<16;      >   //We have to shift 16bit to move sector_start_address
         *      while (RCIF != 1);
         *      Amount_of_erase_sector_OBC = RCREG;          //Receive by UBYTE ex.) 5Å®0x05, 10Å®0x0a, 15Å®0x0f
         *      for (UBYTE i=0x00; i<Amount_of_erase_sector_OBC; i++){     > 
         *          flash_Erase(FROM_sector_adr, S_ERASE);
         *          FROM_sector_adr += 0x10000;                //Jump to next sector which you want to delete
         *          if(i % 0x02 == 0x01){
         *              CLRWDT();
         *              WDT_CLK = ~WDT_CLK;
         *          }
         *      }
         * }
         * ======================================================================================
        }*/
        
        /* Comment
         * ======================================================================================
         * Make initialize mode
            Only copy the first regulation above and paste
         * ======================================================================================
         * Code
         * ======================================================================================
         * else if(Command == 'I'){     //Initialize mode
         * 
         *   init_mpu();
         *   //initbau(BAU_HIGH);                //115200bps
         *   //initbau(BAU_MIDDLE);              //57600bps
         *   initbau(BAU_LOW);                   //14400bps
         *   MAX2828_EN = 1;                     //MAX2828 ON
         *    __delay_us(100);                    //100us wait
         *    FLASH_SPI_EI();                     //enable SPI
         *   init_max2828();                     //init MAX2828
         *   Mod_SW = 0;                         //FSK modulation ON
         * ======================================================================================
        }*/
        
        /* Comment
         * ======================================================================================
         *  Make Change Roop_adr received from OBC
         *  Add Command C:Change Roop_adr when some sectors of FROM are broken
         *  Receive a part of tmp_adr_change of FROM and overwrite Roop_adr
         *  Ground Station can choose only sector start address kind of 0x00ÅõÅõ0000
         * ======================================================================================r
         *Code
         * ======================================================================================
         *else if(Command == 'C'){
         *  while(RCIF != 1);
         *  FROM_sector_adr = (UDWORD)RCREG
         *  FROM_sector_adr = FROM_sector_adr<<16;       >   //bit shift and clear low under 4bit for next 4bit address
         *  Roop_adr = FROM_sector_adr;
         * ======================================================================================
         */
        /* Comment
         * ======================================================================================
         * Make Sleep mode (Command =='S')
         * We make PIC sleep mode. All pins are low without MCLR pin in order to save energy.
         * We have to keep MCLR pin High.
         * Above this is uncorrect because we shouldn't use PIC_SLEEP. 
         * Sleep mode only FROM, Max2828, Amp
         *=======================================================================================
         * Code
         * ======================================================================================
         *else if(Command == 'S')
         *  flash_Deep_sleep();
         *  sleep_Max2828   There is no SHDN pin from PIC so we may not mak e sleep mode
         *  offAmp();
         * ======================================================================================
         */
        /*Comment
         * ======================================================================================
         * Make Wake up mode (Command == 'W')
         *======================================================================================
         * Code
         * ======================================================================================
         *else if(Command == 'W){
         *  flash_Wake_up();
         *  offAmp();   //This is in Wakeup mode but we don't have to make Amp on.
         * }
         * ======================================================================================
         */
        else
        {
            offAmp();
        }
    }
}