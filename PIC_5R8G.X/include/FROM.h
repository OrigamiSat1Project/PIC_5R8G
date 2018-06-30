#ifndef __FROM_H__
#define __FROM_H__

#define MTL_MCU_LITTLE									/* リトルエンディアン					*/

/*------- Define the serial FLASH device -------------*/
//#define M25P05A										/* 512kbit	( 64kByte)					*/	/** SET **/
//#define M25P10A										/* 1Mbit	( 128kByte)					*/	/** SET **/
//#define M25P20										/* 2Mbit	( 256kByte)					*/	/** SET **/
//#define M25P40										/* 4Mbit	( 512kByte)					*/	/** SET **/
//#define M25P16										/* 16Mbit	( 2MByte)					*/	/** SET **/
#define M25P32										/* 32Mbit	( 4MByte)					*/	/** SET **/
//#define M25P64										/* 64Mbit	( 8MByte)					*/	/** SET **/

/*----------- Definitions of return value ------------*/
#define FLASH_OK				(UBYTE)(1)			/* Successful operation						*/
#define FLASH_ERR				(UBYTE)(0)			/* error									*/

/*------------ Definitions of erase type  ------------*/
#define B_ERASE					(UBYTE)0x00			/*Bulk erase								*/
#define S_ERASE					(UBYTE)0x01			/*Sector erase								*/

/*-------- Definitions of device information ---------*/

#ifdef M25P05A
#define FLASH_MEM_SIZE			(UDWORD)65536		/*  64kByte	(512kbit)						*/
#define FLASH_SECT_ADDR			(UDWORD)0xffff8000	/* Sector address setting					*/
#define FLASH_WPAG_SIZE			(UDWORD)256			/* Page size when writing					*/
#define FLASH_ADDR_SIZE			(UBYTE)3			/* Address size(Byte)						*/
#define FLASH_WP_WHOLE_MEM		(UBYTE)0x03			/* Whole memory WP setting					*/
#endif	/* #ifdef M25P05A */

#ifdef M25P10A
#define FLASH_MEM_SIZE			(UDWORD)131072		/*  128kByte	(1Mbit)						*/
#define FLASH_SECT_ADDR			(UDWORD)0xffff8000	/* Sector address setting					*/
#define FLASH_WPAG_SIZE			(UDWORD)256			/* Page size when writing					*/
#define FLASH_ADDR_SIZE			(UBYTE)3			/* Address size(Byte)						*/
#define FLASH_WP_WHOLE_MEM		(UBYTE)0x03			/* Whole memory WP setting					*/
#endif	/* #ifdef M25P10A */

#ifdef M25P20
#define FLASH_MEM_SIZE			(UDWORD)262144		/*  256kByte	(2Mbit)						*/
#define FLASH_SECT_ADDR			(UDWORD)0xffff0000	/* Sector address setting					*/
#define FLASH_WPAG_SIZE			(UDWORD)256			/* Page size when writing					*/
#define FLASH_ADDR_SIZE			(UBYTE)3			/* Address size(Byte)						*/
#define FLASH_WP_WHOLE_MEM		(UBYTE)0x03			/* Whole memory WP setting					*/
#endif	/* #ifdef M25P20 */

#ifdef M25P40
#define FLASH_MEM_SIZE			(UDWORD)524288		/*  512kByte	(4Mbit)						*/
#define FLASH_SECT_ADDR			(UDWORD)0xffff0000	/* Sector address setting					*/
#define FLASH_WPAG_SIZE			(UDWORD)256			/* Page size when writing					*/
#define FLASH_ADDR_SIZE			(UBYTE)3			/* Address size(Byte)						*/
#define FLASH_WP_WHOLE_MEM		(UBYTE)0x07			/* Whole memory WP setting					*/
#endif	/* #ifdef M25P40 */

#ifdef M25P16
#define FLASH_MEM_SIZE			(UDWORD)2097152		/*  2MByte	(16Mbit)						*/
#define FLASH_SECT_ADDR			(UDWORD)0xffff0000	/* Sector address setting					*/
#define FLASH_WPAG_SIZE			(UDWORD)256			/* Page size when writing					*/
#define FLASH_ADDR_SIZE			(UBYTE)3			/* Address size(Byte)						*/
#define FLASH_WP_WHOLE_MEM		(UBYTE)0x07			/* Whole memory WP setting					*/
#endif	/* #ifdef M25P16 */

#ifdef M25P32
#define FLASH_MEM_SIZE			(UDWORD)4194304		/*  4MByte	(32Mbit)						*/
#define FLASH_SECT_ADDR			(UDWORD)0xffff0000	/* Sector address setting					*/
#define FLASH_WPAG_SIZE			(UDWORD)256			/* Page size when writing					*/
#define FLASH_ADDR_SIZE			(UBYTE)3			/* Address size(Byte)						*/
#define FLASH_WP_WHOLE_MEM		(UBYTE)0x07			/* Whole memory WP setting					*/
#endif	/* #ifdef M25P32 */

#ifdef M25P64
#define FLASH_MEM_SIZE			(UDWORD)8388608		/*  8MByte	(64Mbit)						*/
#define FLASH_SECT_ADDR			(UDWORD)0xffff0000	/* Sector address setting					*/
#define FLASH_WPAG_SIZE			(UDWORD)256			/* Page size when writing					*/
#define FLASH_ADDR_SIZE			(UBYTE)3			/* Address size(Byte)						*/
#define FLASH_WP_WHOLE_MEM		(UBYTE)0x07			/* Whole memory WP setting					*/
#endif	/* #ifdef M25P64 */

#define FLASH_CMD_SIZE			(UBYTE)1			/* Command size(Byte)						*/
#define FLASH_STSREG_SIZE		(UWORD)1			/* Status register size(Byte)				*/
#define FLASH_IDDATA_SIZE		(UWORD)3			/* ID data size(Byte)						*/
#define FLASH_ESIG_SIZE			(UWORD)1			/* Electronic Signature size(Byte)			*/

/*----------------------------------------------------------------------------------------------*/
/*	 Define the control port.			 														*/
/*----------------------------------------------------------------------------------------------*/
#define FROM_Dout			RC5						/* FLASH DataOut							*/	/** SET **/
#define FROM_Din			RC4						/* FLASH DataIn								*/	/** SET **/
#define FROM_SCLK			RC3						/* FLASH CLK								*/	/** SET **/
#define FROM_nCS			RB2						/* FLASH CS0	(Negative-true logic)		*/	/** SET **/

/*----------------------------------------------------------------------------------------------*/
/*   Define the software timer value of erase busy waiting.										*/
/*   If you want to wait till the flash comes to ready status without time out,					*/
/*   comment the definition FLASH_EBUSY_WAIT_TIME.												*/
/*----------------------------------------------------------------------------------------------*/
//#define FLASH_EBUSY_WAIT_TIME																		/** SET **/

#ifdef FLASH_EBUSY_WAIT_TIME
#define FLASH_EBUSY_WAIT		(UWORD)40000	/* Erase busy waiting time	 40000* 1Ms =   40s	*/	/** SET **/
#else
#define FLASH_EBUSY_WAIT		(UWORD)0		/* Waiting without time out						*/
#endif

/*----------- Definitions of port control ------------*/
#define FLASH_HI			0b1						/* Port "H"									*/
#define FLASH_LOW			0b0						/* Port "L"									*/

/*------- Definitions of software timer value --------*/
/* Transmit&receive waiting time */
#define FLASH_SPI_TX_WAIT	(UWORD)50000	/* UART transmission completion waiting time	50000* 1us =  50ms	*/
#define FLASH_SPI_RX_WAIT	(UWORD)50000	/* UART receive completion waiting time			50000* 1us =  50ms	*/
#define FLASH_DMA_TX_WAIT	(UWORD)50000	/* DMA transmission completion waiting time		50000* 1us =  50ms	*/
#define FLASH_DMA_RX_WAIT	(UWORD)50000	/* DMA receive completion waiting time			50000* 1us =  50ms	*/
#define FLASH_WBUSY_WAIT	(UWORD)18000	/* Write busy waiting time						18000* 1us =  18ms	*/

/* Access time */
#define FLASH_T_UART_WAIT	(UWORD)1				/* UART送受信完了待ち時間                       */
#define FLASH_T_DMA_WAIT	(UWORD)1				/* DMA(Direct Memory Access)送受信完了待ち時間  */
#define FLASH_T_WBUSY_WAIT	(UWORD)1				/* 書き込みbusy完了待ち時間                     */
#define FLASH_T_EBUSY_WAIT	(UWORD)1				/* 消去busy完了待ち時間                         */
#define FLASH_T_DP_WAIT		(UWORD)4				/* Deep standby（節電）完了待ち時間             */
#define FLASH_T_RES_WAIT	(UWORD)30				/* Deep standby（節電）開放待ち時間             */
#define FLASH_T_CS_HOLD		(UWORD)1				/* CS安定化待ち時間                             */
#define FLASH_T_R_ACCESS	(UWORD)1				/* 読み込み開始待ち時間                         */

/*---------- Definitions of FLASH command -----------*/
#define FLASH_CMD_WREN		(UBYTE)0x06				/* 書き込み許可             						*/
#define FLASH_CMD_WRDI		(UBYTE)0x04				/* 書き込み不可             						*/
#define FLASH_CMD_RDSR		(UBYTE)0x05				/* 状態レジスタ読み込み							*/
#define FLASH_CMD_WRSR		(UBYTE)0x01				/* 状態レジスタ書き込み     						*/
#define FLASH_CMD_READ		(UBYTE)0x03				/* メモリアレイ読み込み     						*/
#define FLASH_CMD_WRITE		(UBYTE)0x02				/* メモリアレイ書き込み     						*/
#define FLASH_CMD_SE		(UBYTE)0xd8				/* SectorErase              					*/
#define FLASH_CMD_BE		(UBYTE)0xc7				/* BlkErace                 					*/
#define FLASH_CMD_DP		(UBYTE)0xb9				/* Deep Power Down								*/
#define FLASH_CMD_RES		(UBYTE)0xab				/* Release Deep Power Down						*/
#define FLASH_CMD_RDID		(UBYTE)0x9f				/* 識別読み込み         							*/

/*------- Definitions of status register value -------*/
#define FLASH_REG_SRWD		(UBYTE)0x80				/* Status Register Write Disable				*/
#define FLASH_REG_BP1		(UBYTE)0x08				/* Block Protect Bit1							*/
#define FLASH_REG_BP0		(UBYTE)0x04				/* Block Protect Bit0							*/
#define FLASH_REG_WEL		(UBYTE)0x02				/* Write Enable Latch Bit						*/
#define FLASH_REG_WIP		(UBYTE)0x01				/* Write In Progress Bit						*/

/*----------------- SPI definitions -----------------*/
#define FLASH_SPI_BUF			SSPBUF					/* SPI バッファレジスタ						*/
#define FLASH_SPI_IF			SSPIF					/* SPI 割り込みフラグ						*/
#define FLASH_SPI_BF			BF						/* SPI バッファフル検出フラグ   				*/

#define FLASH_DUMMY_DATA		(UBYTE)0xFF				/* SPI ダミーデータ                     		*/

/************************************************************************************************/
/*	Macros																						*/
/************************************************************************************************/

/* SPI通信許可 */
#define FLASH_SPI_EI() do {		\
	SSPSTAT = (UBYTE)0x40;		\
	SSPCON  = (UBYTE)0x20;		\
} while (0)

/* CSコントロール */
#define FLASH_SET_CS(Lv) do {	\
	FROM_nCS = Lv;				\
} while (0)
//  Cmd = Command, Arg = Argument, CmdSize = Command Size
/*--------- Command transmission processing ----------*/
/*											 Cmd					Arg				CmdSize			*/
#define flash_Cmd_WREN()		flash_Send_Cmd(FLASH_CMD_WREN,	(UDWORD)0,		FLASH_CMD_SIZE		)
#define flash_Cmd_RDSR()		flash_Send_Cmd(FLASH_CMD_RDSR,	(UDWORD)0,		FLASH_CMD_SIZE		)
#define flash_Cmd_READ(Arg)		flash_Send_Cmd(FLASH_CMD_READ,	(UDWORD)Arg,	FLASH_CMD_SIZE+FLASH_ADDR_SIZE	)
#define flash_Cmd_WRITE(Arg)	flash_Send_Cmd(FLASH_CMD_WRITE,	(UDWORD)Arg,	FLASH_CMD_SIZE+FLASH_ADDR_SIZE	)
#define flash_Cmd_BE()			flash_Send_Cmd(FLASH_CMD_BE,	(UDWORD)0,		FLASH_CMD_SIZE		)
#define flash_Cmd_SE(Arg)		flash_Send_Cmd(FLASH_CMD_SE,	(UDWORD)Arg,	FLASH_CMD_SIZE+FLASH_ADDR_SIZE	)

UBYTE flash_Write_Page(UDWORD , UWORD , UBYTE *);	//1-page write processing
UBYTE flash_Read_StsReg(UBYTE *);					//Write-protection check
UBYTE flash_Erase(UDWORD, UBYTE);					//Erase processing
UBYTE flash_Read_Data(UDWORD, UDWORD, UBYTE *);		//Data read processing
UBYTE flash_Write_Data(UDWORD, UDWORD, UBYTE *);	//Data write processing
void flash_ExchgLong(UDWORD);						//エンディアンの変換関数
 
UBYTE flash_SPI_DataOut(UWORD, UBYTE *);
UBYTE flash_Write_En(void);
UBYTE flash_Send_Cmd(UBYTE , UDWORD , UBYTE );

#endif /* __FROM_H__ */