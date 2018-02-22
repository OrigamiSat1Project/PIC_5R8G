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
#define MaxOfMemory 20  //  TODO : Use Bank function then magnify buffer size
const UBYTE FooterOfJPEG[] =  {'R', 'A'}; 

void main(void){
    UDWORD			g1_data_adr = (UDWORD)0x00010000;
    UDWORD			g2_data_adr = (UDWORD)0x00020000;

    UBYTE Rxdata[MaxOfMemory];
    UBYTE Txdata[MaxOfMemory];
    UINT indexOfRxdata = 0;
    UINT indexOfTxdata = 0;

    UDWORD FROM_Write_adr = g1_data_adr;
    UDWORD FROM_Read_adr  = g1_data_adr;
    UDWORD Roop_adr       = g1_data_adr;
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
        //TXEN = Bit_Low;
        TXEN = Bit_High;
        UBYTE Command;
        while(RCIF != 1);
        Command = RCREG;
        if(Command == 'P')
        {
            sendChar('P');
            while(CAM2 == 1);   //  wait 5V SW
            if(CAMERA_POW == 1){
                //onAmp();
            }
            FROM_Read_adr = Roop_adr;
            UINT sendBufferCount = 0;
            //CREN = Bit_Low;
            //TXEN = Bit_High;
            while(FROM_Read_adr < FROM_Write_adr && CAM2 == 0){
                flash_Read_Data(FROM_Read_adr, (UDWORD)(MaxOfMemory), &Txdata);
                if(sendBufferCount % JPGCOUNT == 0){
                    __delay_ms(3000);
                    send_01();  //  send preamble
                }
                for(UINT i=0;i<20;i++){
                    sendChar(Txdata[i]);
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
            send_OK();
        }
        else if (Command == 'D')
        {
            while(CAM2 == 1);   //  wait 5V SW
            while(CAM2 == 0){
                if(CAMERA_POW == 1){
                    //onAmp();
                }
                send_dummy_data();
            }
        }
        else if (Command == 'R')
        {
            sendChar('R');
            flash_Erase(g1_data_adr,S_ERASE);    //delete memory of g1_data_adr sector ( 65536byte )
            flash_Erase(g2_data_adr,S_ERASE);    //delete memory of g2_data_adr sector ( 65536byte )
            Roop_adr = g1_data_adr;
            FROM_Write_adr = Roop_adr;
            UBYTE receiveEndJpegFlag = 0x00;
            while(receiveEndJpegFlag != 0x11){
                for (UINT i = 0; i < MaxOfMemory; i++) {
                    while (RCIF != 1);
                    Rxdata[i] = RCREG;
                    if(receiveEndJpegFlag == 0x00 && RCREG == FooterOfJPEG[0]){
                        receiveEndJpegFlag = 0x01;
                    }else if (receiveEndJpegFlag == 0x01 && RCREG == FooterOfJPEG[1]){
                        //  get JPEG footer
                        receiveEndJpegFlag = 0x11;
                        //  save data to FROM before break roop
                        flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Rxdata);
                        FROM_Write_adr += (UDWORD)(MaxOfMemory);
                        break;
                    }else{
                        receiveEndJpegFlag = 0x00;
                    }
                    sendChar(RCREG);
                }
                flash_Write_Data(FROM_Write_adr, (UDWORD)(MaxOfMemory), &Rxdata);
                FROM_Write_adr += (UDWORD)(MaxOfMemory);
            }
            send_OK();
        }
        else
        {
            offAmp();
        }
    }
//
//        // Ampポートに設定
//        /**/
//        CAMERA_POW = 0;
//        CAMERA_SEL = 0;
//        max2828_txon();
//        CREN = Bit_Low;
//        TXEN = Bit_High;
//        delay_ms(10);
//
//        //  読み込みアドレスをループアドレスにセット
//        FROM_Read_adr = Roop_adr;
//        //  プリアンブル送信
//        send_01();
//        //  EEPROMから取り出し→UART送信
//        for(UINT j=0;j<JPGCOUNT;j++){
//            UINT sendcount = 0;
//            flash_Read_Data(FROM_Read_adr,20UL,&Txdata);
//
//            for(UINT i=0;i<20;i++){
//                sendChar(Txdata[i]);
//                //sendChar(0x00);
//                __delay_us(20);
//            }
//            FROM_Read_adr += 20UL;
//            /*
//            sendcount ++;
//            if(sendcount == 60){
//                __delay_ms(50);
//                sendcount = 0;
//            }*/
//        }
//        send_01();
//        sendChar('\r');
//        sendChar('\n');
//        g1_data_adr += (UDWORD)0x00020000;
//        g2_data_adr += (UDWORD)0x00020000;
//
//        //  外付けWDT有効関数
//        /*
//        roopcount++;
//        if(roopcount == 29)
//        {
//            //CLRWDT();
//            //WDT_CLK = ~WDT_CLK;
//            //  メモリ，カウンタリセット
//            roopcount = 0;
//            g1_data_adr = (UDWORD)0x00010000;
//            //g2_data_adr = (UDWORD)0x00020000;
//        }*/
//    }
}
