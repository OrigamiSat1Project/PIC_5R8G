#include <xc.h>
//#include "pic16f886.h"

#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"
#include "SECTOR.h"
#include "ReceiveJPEG.h"
#include "Downlink.h"
#include "CRC16.h"
#include "Timer.h"
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
const UBYTE MaxOfSector = 0x3f;

#define JPGCOUNT 5000
void main(void){
    UDWORD			g1_data_adr = (UDWORD)0x00010000;

    UDWORD Roop_adr = g1_data_adr;
    UDWORD Jump_adr = 0x20000;
   /* Comment
    * =====================================================
    * We save Roop_adr in FROM's 0 sector
    * Roop_adr in 0x00000000
    * Jump_adr in 0x00000001
    * =====================================================
    */
    init_module();
    while(1){
        if(CAMERA_POW == 0){
            offAmp();
        }
        CREN = Bit_High;
        TXEN = Bit_High;
        UBYTE Command[8];
        Command[0] = 0x21;
        while(crc16(0,Command,6) != CRC_check(Command, 6)){
            for(UINT i=0;i<8;i++){
                Command[i] = 0x21;
            }
            //  sync with commands by OBC
            while(Command[0] != '5'){
                Command[0] = getUartData(0x00);
            }
            for(UINT i=1;i<8;i++){
                Command[i] = getUartData('T');
            }
        }
        //TODO : busy signal when succeed receive command
//        BUSY = 0;
//        __delay_ms(3000);
//        BUSY = 1;
        UINT ECC_length = 0;

        switch(Command[1]){
            case 'P':
                switch(Command[2]){
                    case '2':   //CAM2
                        switch(Command[3]){
                            case 'T':
                                Downlink(Roop_adr, Jump_adr, 0x01);
                                break;
                            case '8':
                                Downlink(Roop_adr, Jump_adr, Command[3]);
                                break;
                            default:
                                break;
                        }
                    case 'C':   //Clock
                        set_timer_counter(0);
                        set_timer_counter_min(0);
                        while(get_timer_counter_min() < (UINT)Command[4]);
                        Downlink_clock(Roop_adr, Jump_adr, Command[3], (UINT)Command[5]);
                        break;
                    default:
                        break;
                }
                break;
            case 'D':
                switch(Command[2]){
                    case 'C':   //Clock
                        set_timer_counter(0);
                        set_timer_counter_min(0);
                        while(get_timer_counter_min() < (UINT)Command[4]);
                        set_timer_counter_min(0);
                        if(CAMERA_POW == 1){
                            onAmp();
                        }
                        send_dummy_data_timer(Command[5]);
                        offAmp();
                        break;
                    case '2':    //Use CAM2
                        while(CAM2 == 1);   //  wait 5V SW
                        while(CAM2 == 0){
                            if(CAMERA_POW == 1){
                                onAmp();
                            }
                            send_dummy_data();
                        }
                        offAmp();
                        break;
                    default:
                        break;
                }
                break;
            case 'R':
                switch(Command[2]){
                    case 'T':
                        Receive_thumbnail_JPEG(Roop_adr, Jump_adr);
                        break;
                    case '8':
                        switch(Command[3]){
                            case 'J':
                                Receive_8split_JPEG(Roop_adr, Jump_adr);
                                break;
                            case 'H':
                                Receive_8split_H264(Roop_adr, Jump_adr);
                                break;
                            default:
                                break;
                        }
                        break;
                    case 'E':
                        for(UINT i=0; i<3; i++){
                            ECC_length += Command[i+3] << 8*(2-i);
                        }
                        Receive_ECC(Roop_adr, Jump_adr, ECC_length);
                        break;
                    case 'C':   //Clock mode
                        set_timer_counter(0);
                        set_timer_counter_min(0);
                        while(get_timer_counter_min() < (UINT)Command[3]);
                        Receive_8split_clock(Roop_adr, Jump_adr,(UINT)Command[4], (UINT)Command[5]);
                        break;
                    default:
                        break;
                }
                break;
            case 'E':
                if(Command[2] + Command[3] > MaxOfSector) break;
                Erase_sectors(Command[2], Command[3]);
                break;
            case 'I':
                init_module();
                break;
            case 'C':
                switch(Command[2]){
                    /* Comment
                     * =========================================================
                     * We can use 63 sectors in FROM
                     * Original JPEG uses 24(3jump * 8groups) sectors each.
                     * Max Roop_adr = 63- 24 = 39(0x27)
                     * Min Jump_adr = 3
                     * Max Jump_adr = 63/8 ~= 7
                     * 63 = 0x3f
                     * =========================================================
                     */
                    case 'R':
                        if(Command[3] == 0x00) break;
                        if((Command[3] + (UBYTE)(Jump_adr>>16) * 8) > MaxOfSector) break;
                        Roop_adr = (UDWORD)Command[3]<<16;
                        break;
                    case 'J':
                        if(Command[3] <= 0x01) break;
                        if(((UBYTE)(Roop_adr>>16) + Command[3] * 8) > MaxOfSector) break;
                        Jump_adr = (UDWORD)Command[3]<<16;
                        break;
                    case 'B':
                        if((Command[3] != BAU_LOW ) &&
                           (Command[3] != BAU_MIDDLE) &&
                           (Command[3] != BAU_HIGH)){
                            break;
                        }
                        change_downlink_baurate(Command[3]);
                        break;
                    case 'D':
                        if(Command[3] >=  0x14) break;
                        set_downlink_time((UINT)Command[3]);
                        break;
                    case 'T':
                        if(Command[3] < 0x05) break;    // break under 5s
                        set_rest_time((UINT)Command[3]);
                        break;
                    default:
                        break;
                }
                break;
            case 'B':
                BUSY = 0;
                __delay_ms(3000);
                BUSY = 1;
                break;
            default:
                offAmp();
                break;
        }
    }
}
