#include <xc.h>
//#include "pic16f886.h"

#include "stdio.h"
#include "string.h"
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"
//#include "stdint.h"

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
const UBYTE FooterOfJPEG[] =  {0xff, 0x0e}; 

/*  How to use receiveEndJpegFlag
 * ===============================================================================================================
 *  Bit7            Bit6            Bit5        Bit4        Bit3        Bit2        Bit1                Bit0
 *  8split_End      8split_cnt2     cnt1        cnt0        ----        ----        Flag_0x0E           Flag_0xFF
 *  
 *  Initialize                  = 0x00
 *  Detect JPEG marker 0xFF     = 0x01
 *  End of receive JPEG         = 0x03  (0b00000011)
 *  During writing sector 2     = 0x10  (0b00010000)
 *  During writing sector 8     = 0x70  (0b01110000)
 *  After 8split_end            = 0x80  (0b10000000)
 * ===============================================================================================================
 */

void main(void){
    UDWORD			g1_data_adr = (UDWORD)0x00010000;
    //UDWORD			g2_data_adr = (UDWORD)0x00020000;   No need

    UBYTE Buffer[MaxOfMemory];
    UINT indexOfBuffer = 0;

    UDWORD FROM_Write_adr = g1_data_adr;
    UDWORD FROM_Read_adr  = g1_data_adr;
    //UDWORD FROM_sector_adr = g1_data_adr;       //Each sector's first address kind of 0x00ÅõÅõ0000. Use in 'C' and 'D' command
    UDWORD Roop_adr = g1_data_adr;
    UDWORD FROM_Jump_next_sector = 0x10000;
    //UINT roopcount = 0;

    init_module();

    while(1){
        if(CAMERA_POW == 0){
            offAmp();
        }
        CREN = Bit_High;
        TXEN = Bit_Low;
        UBYTE Command[8];
        
        do{
            for(UINT i=0;i<8;i++){
                while(RCIF != 1);
                Command[i] = RCREG;
                if(Command[i] == 0xff) break;
            }
        }while(Command[0] != '5');

        //  TODO : Add time restrict of picture downlink (10s downlink, 5s pause)
        
        switch(Command[1]){
            case 'P':
                sendChar('P');
                while(CAM2 == 1);   //  wait 5V SW
                FROM_Read_adr = Roop_adr;
                UINT sendBufferCount = 0;
                //UINT readFROM_Count = 0;                //How many sectors did you read in this while statement.
                //UBYTE Identify_8split = 0xff;           //Which sectors do you want to downlink. This is a kind of Flag. Defualt: all sectors(8)
                /*  How to use Identify_8split
                 * ===========================================================================================
                 *  Bit7        Bit6        Bit5        Bit4        Bit3        Bit2        Bit1        Bit0
                 *  8/8         7/8         6/8         5/8         4/8         3/8         2/8         1/8
                 * ===========================================================================================
                 */            
                CREN = Bit_Low;
                TXEN = Bit_High;
                while(FROM_Read_adr < FROM_Write_adr && CAM2 == 0){
                    /* Comment
                     * =============================================================
                     * Which sectors do you read by reading Identify_8split.
                     * Flag is ON : flash_Read_Data
                     * Flag is OFF : FROM_Read_adr is jumping to next sector's start address.
                     * =============================================================
                     * Code
                     * =============================================================
                     * while(RCIF != 1);
                     * Identify_8split = Command[2];
                     * if(Identify_8split & (0x01<<readFROM_Count)){    >
                     *      flash_Read_Data(FROM_Read_adr, (UDWORD)(MaxOfMemory), &Buffer);
                     * }
                     * else{
                     *      FROM_Read_adr &= ~0xffff;            //Clear under lower 2bytes
                     *      FROM_Read_adr += 0x10000;           //Jump to next sector's address
                     * }
                     * readFROM_Count +=1;
                     * =============================================================
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
                break;
            case 'D':
                while(CAM2 == 1);   //  wait 5V SW
                while(CAM2 == 0){
                    if(CAMERA_POW == 1){
                        onAmp();
                    }
                    send_dummy_data();
                }
                offAmp();
                send_OK();
                break;
            case 'R':
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
                break;
            case 'E':
               /* Comment
                * ======================================================================================
                * Make Erase sector mode
                * We have to delete sectors in order to rewrite data in the sectors.
                * Bulk Erase takes long time (10s) so we should use sector erase.
                * Receive sector's identificate address and how many sectors want to delete, delete sectors from received sector.
                * ======================================================================================
                * Code
                * ======================================================================================
                * Erase_sectors(Command[2], Command[3]);
                * ======================================================================================
                */
                break;
            case 'I':
                init_module();
                break;
            case 'C':
               /* Comment
                * ======================================================================================
                *  Make Change Roop_adr received from OBC
                *  Add Command C:Change Roop_adr when some sectors of FROM are broken
                *  Receive a part of tmp_adr_change of FROM and overwrite Roop_adr
                *  Ground Station can choose only sector start address kind of 0x00ÅõÅõ0000
                * ======================================================================================
                *Code
                * ======================================================================================
                *  Roop_adr = (UDWORD)Command[2]<<16;       >   //bit shift and clear low under 4bit for next 4bit address
                * ======================================================================================
                */
                break;
            case 'S':
               /* Comment
                * ======================================================================================
                * Make Sleep mode (Command =='S')
                * We make PIC sleep mode. All pins are low without MCLR pin in order to save energy.
                * We have to keep MCLR pin High.
                * Above this is uncorrect because we shouldn't use PIC_SLEEP. 
                * Sleep mode only FROM, Amp
                *=======================================================================================
                * Code
                * ======================================================================================
                *  flash_Deep_sleep();
                *  offAmp();
                * ======================================================================================
                */
                break;
            case 'W':
               /*Comment
                * ======================================================================================
                * Make Wake up mode (Command == 'W')
                *======================================================================================
                * Code
                * ======================================================================================
                *  flash_Wake_up();
                * ======================================================================================
                */
                break;
            default:
                offAmp();
                break;
        }
    }
}