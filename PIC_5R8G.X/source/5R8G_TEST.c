#include <xc.h>
//#include "pic16f886.h"

#include "stdio.h"
#include "string.h"
#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"
#include "SECTOR.h"
#include "ReceiveJPEG.h"
#include "Downlink.h"
#include "CRC16.h"
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
void main(void){
    UDWORD			g1_data_adr = (UDWORD)0x00010000;
    //UDWORD			g2_data_adr = (UDWORD)0x00020000;   No need

    //UBYTE Buffer[MaxOfMemory];
    //UINT indexOfBuffer = 0;

    //UDWORD FROM_Write_adr = g1_data_adr;
    //UDWORD FROM_Read_adr  = g1_data_adr;
    //UDWORD FROM_sector_adr = g1_data_adr;       //Each sector's first address kind of 0x00??申?申??申?申??申?申??申?申0000. Use in 'C' and 'D' command
    UDWORD Roop_adr = g1_data_adr;
    UDWORD Jump_adr = 0x020000;
    //UDWORD FROM_Jump_next_sector = 0x10000;
    //UINT roopcount = 0;
    init_module();
    while(1){
        if(CAMERA_POW == 0){
            offAmp();
        }
        CREN = Bit_High;
        TXEN = Bit_High;
        UBYTE Command[8];
        Command[0] = 0x21;

        while(Identify_CRC16(Command) != CRC_check(Command, 6)){
            for(UINT i=0;i<8;i++){
                Command[i] = 0x21;
            }
            //  sync with commands by OBC
            while(Command[0] != '5'){
                while(RCIF != 1);
                Command[0] = RCREG;
            }
            //  TODO : Add time restrict
            for(UINT i=1;i<8;i++){
                while(RCIF != 1);
                Command[i] = RCREG;
            }
        }
        for(UINT i=0;i<8;i++){
            sendChar(Command[i]);
        }
        //FIXME : debug
        send_OK();

        switch(Command[1]){
            case 'P':
                switch(Command[2]){
                    case '8':
                        Downlink(Roop_adr, Jump_adr, Command[3]);
                        break;
                    case 'T':
                        Downlink(Roop_adr, Jump_adr, 0x01);
                        break;
                }
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
                //  FIXME : for debug
                send_OK();
                break;
            case 'R':
                switch(Command[2]){
                    case '8':
                        Receive_8split_JPEG(Roop_adr, Jump_adr);
                        send_OK();
                        break;
                    case 'T':
                        Receive_thumbnail_JPEG(Roop_adr, Jump_adr);
                        send_OK();
                        break;
                    default:
                        break;
                }
                break;
            case 'E':
                Erase_sectors(Command[2], Command[3]);
                break;
            case 'I':
                init_module();
                break;
            case 'C':
                switch(Command[2]){
                    case 'R':
                        //  sector size limit
                        if(Command[3] >= 0x47){
                            Command[3] = 0x45;
                        }
                        Roop_adr = (UDWORD)Command[3]<<16;
                        //FIXME : debug
                        sendChar((UBYTE)(Roop_adr >> 16));
                        send_OK();
                        break;
                    case 'J':
                        if(Command[3] >= 0x08){
                            Command[3] = 0x07;
                        }
                        Jump_adr = (UDWORD)Command[3]<<16;
                        break;
                    case 'B':
                        if((Command[3] != BAU_LOW ) &&
                           (Command[3] != BAU_MIDDLE) &&
                           (Command[3] != BAU_HIGH)){
                            break;
                        }
                        change_downlink_baurate(Command[3]);
                        send_OK();
                        break;
                    default:
                        break;
                }
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
                *=======================================================================================
                flash_Deep_sleep();
                offAmp();
                break;
                * =======================================================================================
                */
            case 'W':
               /*Comment
                * ======================================================================================
                * Make Wake up mode (Command == 'W')
                *======================================================================================
                * Code
                * =======================================================================================
                * flash_Wake_up();
                * =======================================================================================
                */
                break;
            default:
                //  FIXME : for debug
                sendChar(0xff);
                offAmp();
                break;
        }
    }
}
