#include <xc.h>
//#include "pic16f886.h"

#include "InitMPU.h"
#include "MAX2828.h"
#include "UART.h"
#include "time.h"
#include "FROM.h"

/*コンフィギュレーションビットの設定*/
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
    UDWORD			g1_data_adr = (UDWORD)0x00010000;	//FROM上のアドレス(データ用)
    UDWORD			g2_data_adr = (UDWORD)0x00020000;	//FROM上のアドレス(データ用)
    
    UBYTE Rxdata[20];
    UBYTE Txdata[20];
    UDWORD FROM_Write_adr = g1_data_adr; //EEPROM書き込み用アドレス
    UDWORD FROM_Read_adr  = g1_data_adr; //EEPROM読み込み用アドレス
    UDWORD Roop_adr       = g1_data_adr; //EEPROMループ用アドレス
    UINT roopcount = 0;
    
    init_mpu();                         //ポートの初期化
    //initbau(BAU_HIGH);                  //115200bps
    //initbau(0x1f);                      //57600bps
    initbau(BAU_LOW);                   //14400bps
    MAX2828_EN = 1;                     //全電源ON
    __delay_us(100);                      //100us wait
    FLASH_SPI_EI();                     //SPI通信許可    
    init_max2828();                     //MAX2828の初期設定
    Mod_SW = 0;                         //送信変調（FSK変調）ON
    
    //flash_Erase(g_data_adr,B_ERASE);
    
//    CAMERA_POW = 0;
//    CAMERA_SEL = 0;
//    max2828_txon();
//    send_pn9();            //PN9送信
//    send_55();             //プリアンブル送信
    send_tst_str();        //テスト文字送信
    
    
    while(1){
        flash_Erase(g1_data_adr,S_ERASE);    //g1_data_adrのsector65536byte分削除
        flash_Erase(g2_data_adr,S_ERASE);    //g2_data_adrのsector65536byte分削除
        
        // Cameraポートに設定
        CAMERA_POW = 1;
        CAMERA_SEL = 1;
        CREN = Bit_High;
        TXEN = Bit_Low;
        MAX2828_TXEN = 0;
        PA_SW = 0;
        
        //send_tst_str();   //  テスト文字列送信
        //echo_back();      //　エコーバック（UART&FROM格納）
        
        //  ループアドレスを次の空きバッファにセット
        Roop_adr = g1_data_adr;
        //  書き込みアドレスをループアドレスにセット
        FROM_Write_adr = Roop_adr;
        //send_OK();
        //  UART受信→EEPROMに格納
        for(UINT j=0;j<JPGCOUNT;j++){
            for(UINT i=0;i<20;i++){
                while(RCIF != 1);
                Rxdata[i] = RCREG;
            }
            flash_Write_Data(FROM_Write_adr,20UL,&Rxdata);
            FROM_Write_adr += 20UL;
        }
        //send_NG();
        
        // Ampポートに設定
        /**/
        CAMERA_POW = 0;
        CAMERA_SEL = 0;
        max2828_txon();
        CREN = Bit_Low;
        TXEN = Bit_High;
        delay_ms(10);
        
        //  読み込みアドレスをループアドレスにセット
        FROM_Read_adr = Roop_adr;
        //  プリアンブル送信
        send_01();
        //  EEPROMから取り出し→UART送信
        for(UINT j=0;j<JPGCOUNT;j++){
            UINT sendcount = 0;
            flash_Read_Data(FROM_Read_adr,20UL,&Txdata);
            
            for(UINT i=0;i<20;i++){
                sendChar(Txdata[i]);
                //sendChar(0x00);
                __delay_us(20);
            }
            FROM_Read_adr += 20UL;
            /*
            sendcount ++;
            if(sendcount == 60){
                __delay_ms(50);
                sendcount = 0;
            }*/
        }
        send_01();
        sendChar('\r');
        sendChar('\n');
        g1_data_adr += (UDWORD)0x00020000;
        g2_data_adr += (UDWORD)0x00020000;
        
        //  外付けWDT有効関数
        /*
        roopcount++;
        if(roopcount == 29)
        {
            //CLRWDT();
            //WDT_CLK = ~WDT_CLK;
            //  メモリ，カウンタリセット
            roopcount = 0;
            g1_data_adr = (UDWORD)0x00010000;
            //g2_data_adr = (UDWORD)0x00020000;
        }*/
    }
}