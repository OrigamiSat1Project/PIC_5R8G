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
const UBYTE FooterOfJPEG[] =  {0xff, 0xd9}; 

void main(void){
    UDWORD			g1_data_adr = (UDWORD)0x00010000;
    //UDWORD			g2_data_adr = (UDWORD)0x00020000;   No need

    UBYTE Buffer[MaxOfMemory];
    UINT indexOfBuffer = 0;

    UDWORD FROM_Write_adr = g1_data_adr;
    UDWORD FROM_Read_adr  = g1_data_adr;
    UDWORD Roop_adr = g1_data_adr;
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
            flash_Erase(g1_data_adr,S_ERASE);    //delete memory of g1_data_adr sector ( 65536byte )
            //flash_Erase(g2_data_adr,S_ERASE);    //delete memory of g2_data_adr sector ( 65536byte )
            //FROM_Write_adr = Roop_adr;         //Reset FROM_Write_adr
            UBYTE receiveEndJpegFlag = 0x00;
            sendChar('R');
            while(receiveEndJpegFlag != 0x11){
                for (UINT i = 0; i < MaxOfMemory; i++) {
                    while (RCIF != 1);
                    Buffer[i] = RCREG;
                    if(receiveEndJpegFlag == 0x00 && RCREG == FooterOfJPEG[0]){
                        receiveEndJpegFlag = 0x01;
                        //sendChar('1');
                    }/*else if (receiveEndJpegFlag == 0x01 && RCREG == FooterOfJPEG[1]){    //No need
                        //  get JPEG footer
                        receiveEndJpegFlag = 0x11;
                        //  save data to FROM before break roop
                        flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Buffer);
                        FROM_Write_adr += (UDWORD)(MaxOfMemory);
                        sendChar('2');
                        break;
                    }*/
                    /*  Jump to next sector of FROM after RCREG reveived EOF(0x0E)
                        DEFINE Secotr_start_adr in order to remember first address of each sector
                        DEFINE Jump_next_sector in order to jump to next sector by address += 0x10000
                        Add 0x10000 to Sector_start_adr and make FROM_Write_adr Sector_start_adr
                        Maybe we have to make another receiveEndJpegFlag ex.0x12 for distinct 8split end from full JPEG end*/
                     /*else if (receiveEndJpegFlag == 0x01 && RCREG == FooterOfJPEG[2]){
                        * const UBYTE FooterOfJPEG[2] = 0x0e;
                        * UDWORD Sector_start_adr = (UDWORD)0x00001000;
                        * UDWORD Jump_next_sector = (UDWORD)0x00010000;
                        * 
                        * receiveEndJpegFlag = 0x11;        //Quit researvation JPEG 
                        * //save data before jump to next sector
                        * flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Buffer);
                        * //Jump to next Sector of FROM
                        * Sector_start_adr += Jump_next_sector;
                        * FROM_Write_adr = Sector_start_adr;
                        * break;
                      }*/
                    else{
                        receiveEndJpegFlag = 0x00;
                    }
                }
                flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Buffer);
                FROM_Write_adr += (UDWORD)(MaxOfMemory);
            }
            send_OK();
        }
        /*  Make initialize mode
            Only copy the first regulation above and paste*/
        /*else if(Command == 'I'){     //Initialize mode
            * init_mpu();
            * //initbau(BAU_HIGH);                //115200bps
            * //initbau(BAU_MIDDLE);              //57600bps
            * initbau(BAU_LOW);                   //14400bps
            * MAX2828_EN = 1;                     //MAX2828 ON
            *  __delay_us(100);                    //100us wait
            *  FLASH_SPI_EI();                     //enable SPI
            * init_max2828();                     //init MAX2828
            * Mod_SW = 0;                         //FSK modulation ON
        }*/
        /*  Add Command C:Change FROM_Write_adr when some sectors of FROM are broken
         *  Receive designated address of FROM and overwrite FROM_Writer_adr
         *  DEFINE FROM_Designated_adr
         */
        /*else if(Command == 'C'){
         *  while(RCIF != 1);
         *  FROM_Designated_adr = RCREG;    //Receive specific address of FROM
         * }*/
        /* Make Sleep mode (Command =='S')
         * We make PIC sleep mode. All pins are low without MCLR pin in order to save energy.
         * We have to keep MCLR pin High.
         * Above this is uncorrect because we shouldn't use PIC_SLEEP. 
         * Sleep mode only FROM, Max2828, Amp
         */
        /*else if(Command == 'S')
         *  flash_Deep_sleep();
         *  sleep_Max2828   There is no SHDN pin from PIC so we may not make sleep mode
         *  offAmp();       
         */
        /*Make Wake up mode (Command == 'W')
         */
        /*else if(Command == 'W){
         *  flash_Wake_up();
         *  offAmp();   //This is in Wakeup mode but we don't have to make Amp on.
         * }
         */
        else
        {
            offAmp();
        }
    }
}