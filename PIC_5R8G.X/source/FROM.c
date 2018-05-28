#include <xc.h>
#include "pic16f886.h"
#include <stddef.h>

#include "typedefine.h"
#include "UART.h"
#include "FROM.h"
#include "time.h"


/************************************************************************************************/
/*	Definitions																					*/
/************************************************************************************************/
#define SHORT_SIZE (UDWORD)0x0000ffff

/************************************************************************************************/
/*	Unions																						*/
/************************************************************************************************/


/************************************************************************************************/
/*	Structs																						*/
/************************************************************************************************/
/* Definition of FLASH command structure */

typedef struct
{
	UBYTE	FlashCmd;										/* Command							*/
	UBYTE	FlashAddr[3];									/* Operand							*/
}FLASH_CMD;													/* total 4byte						*/

/************************************************************************************************/
/*	Globals																						*/
/************************************************************************************************/
/*""VAL DEF""*/
static FLASH_CMD			gFlash_CmdBuf;					/* Command transmission buffer		*/
//UBYTE						gFlash_WP;						/* Write-protection state			*/

/************************************************************************************************/
/*	Prototypes																					*/
/************************************************************************************************/
UBYTE flash_Write_En(void);							/* Writing enable					*/
static UBYTE flash_Write_Di(void);							/* Writing disable					*/
static UBYTE flash_Wait_Busy(UWORD BusyTime, UWORD BusyCnt);
															/* Busy completion waiting			*/
UBYTE flash_SPI_DataOut(UWORD, UBYTE *);
static UBYTE flash_Send_Cmd(UBYTE, UDWORD, UBYTE);
static UBYTE flash_SPI_Rx(UWORD, UBYTE *);
UBYTE flash_Read_Data(UDWORD, UDWORD, UBYTE *);
UBYTE flash_Send_Cmd(UBYTE , UDWORD , UBYTE );
UBYTE flash_Deep_sleep(void);
UBYTE flash_Wake_up(void);

/* Write Enable */
UBYTE flash_Write_En(void)
{
	UBYTE				Ret;

	FLASH_SET_CS(FLASH_LOW);								/* CS "L"							*/
	delay_us(FLASH_T_CS_HOLD);								/* 1us wait				*/

	/* Writing enable command transmission */
	Ret = flash_Cmd_WREN();

	delay_us(FLASH_T_CS_HOLD);								/* 1us wait             */
	FLASH_SET_CS(FLASH_HI);									/* CS "H"							*/
	return Ret;
}

/* Write disable */
static UBYTE flash_Write_Di(void)
{
	UBYTE				Ret;

	FLASH_SET_CS(FLASH_LOW);								/* CS "L"							*/
	delay_us(FLASH_T_CS_HOLD);								/* 1us wait						*/

	/* Writing disable command transmission */
	Ret = flash_Cmd_WRDI();

	delay_us(FLASH_T_CS_HOLD);								/* 1us wait						*/
	FLASH_SET_CS(FLASH_HI);									/* CS "H"							*/
	return Ret;
}

/*Waiting for BUSY process*/
static UBYTE flash_Wait_Busy(UWORD BusyTime, UWORD BusyCnt)
{
	UBYTE				RxBuf;								/* Receive temp buffer				*/
	UBYTE				Ret;

	if (BusyCnt == 0)
	{
		while (1)
		{
			Ret = flash_Read_StsReg((UBYTE *)&RxBuf);		/* Status register reading			*/
			if (Ret == FLASH_OK)
			{
				/* Ready/Busy chack */
				if ((RxBuf & FLASH_REG_WIP) == 0x00)		/* Ready ?							*/
				{
					/* Ready */
					return FLASH_OK;
				}
			}
			delay_us(BusyTime);
		}
	}
	else
	{
		UWORD				WaitCnt;						/* Wait counter						*/
		for (WaitCnt = BusyCnt; WaitCnt != 0; WaitCnt--)
		{
			Ret = flash_Read_StsReg((UBYTE *)&RxBuf);		/* Status register reading			*/
			if (Ret == FLASH_OK)
			{
				/* Ready/Busy chack */
				if ((RxBuf & FLASH_REG_WIP) == 0x00)		/* Ready ?							*/
				{
					/* Ready */
					return FLASH_OK;
				}
			}
			delay_us(BusyTime);
		}
	}
	/* Busy */
	return FLASH_ERR;
}

/* エン�?ィアンの変換関数 */
void flash_ExchgLong(UDWORD ChgData)
{
	EXCHG_LONG Tmp;
	Tmp.us = ChgData;

#ifdef MTL_MCU_LITTLE

	gFlash_CmdBuf.FlashAddr[0] = Tmp.uc[2];
	gFlash_CmdBuf.FlashAddr[1] = Tmp.uc[1];
	gFlash_CmdBuf.FlashAddr[2] = Tmp.uc[0];

#else	/* #ifdef MTL_MCU_LITTLE */

	gFlash_CmdBuf.FlashAddr[0] = Tmp.uc[1];
	gFlash_CmdBuf.FlashAddr[1] = Tmp.uc[2];
	gFlash_CmdBuf.FlashAddr[2] = Tmp.uc[3];

#endif	/* #ifdef MTL_MCU_LITTLE */
}

/* SPI送信関数 */
UBYTE flash_SPI_DataOut(UWORD TxCnt, UBYTE * pData)
{
	UWORD				TxWait;								/* Transmission waiting counter			*/
	UBYTE				Rxbuf;
    
	TxWait = FLASH_SPI_TX_WAIT;
	for (; TxCnt != 0; TxCnt--)								/* Loop for transmission byte			*/
	{							
		FLASH_SPI_BUF = *pData;								/* 送信�?ータをセ�?�?						*/

		while (FLASH_SPI_IF == FLASH_LOW)					/* 送信完�?フラグ�?ち						*/
		{
			TxWait--;
			if (TxWait == 0)								/* Transmission waiting time over		*/
			{
				return FLASH_ERR;
			}
			delay_us(FLASH_T_UART_WAIT);
		}
		FLASH_SPI_IF = FLASH_LOW;									/* 送信完�?フラグ消去						*/

		while (FLASH_SPI_BF == FLASH_LOW);							/* 受信完�?フラグ�?ち						*/
		Rxbuf = FLASH_SPI_BUF;									/* 受信�?ータの空読み						*/

		pData++;
	}
	return FLASH_OK;
}

/* コマンド�?�信関数 */
UBYTE flash_Send_Cmd(UBYTE Cmd, UDWORD Arg, UBYTE CmdSize)
{
	/* The specified command is stored in the buffer */
	gFlash_CmdBuf.FlashCmd = Cmd;
	if (CmdSize != FLASH_CMD_SIZE)
	{
		flash_ExchgLong(Arg);
	}
	/* Command transmission */
	return flash_SPI_DataOut((UWORD)CmdSize, (UBYTE *)&gFlash_CmdBuf.FlashCmd);
}

/* 1-page write processing */
UBYTE flash_Write_Page(UDWORD WAddr, UWORD WCnt, UBYTE * pData)
{
	UBYTE				Ret;
	/* Writing enable(WEL set) */
	Ret = flash_Write_En();
	if (Ret != FLASH_OK)
	{
		return Ret;
	}

	FLASH_SET_CS(FLASH_LOW);							/* CS "L"								*/
	delay_us(FLASH_T_CS_HOLD);							/* 1us ウェイ�?							*/

	/* Write command transmission */
	Ret = flash_Cmd_WRITE(WAddr);
	if (Ret != FLASH_OK)
	{
		FLASH_SET_CS(FLASH_HI);							/* CS "H"								*/
		return Ret;
	}

	/* Data writing */
	Ret = flash_SPI_DataOut(WCnt, pData);				/* 1byte is written for speed-up		*/
	if (Ret != FLASH_OK)
	{
		FLASH_SET_CS(FLASH_HI);							/* CS "H"								*/
		return Ret;
	}

	delay_us(FLASH_T_CS_HOLD);							/* 1us ウェイ�?							*/
	FLASH_SET_CS(FLASH_HI);								/* CS "H"								*/

	/* Write busy completion waiting */
	Ret = flash_Wait_Busy(FLASH_T_WBUSY_WAIT, FLASH_WBUSY_WAIT);

	return Ret;
}

/* Data write processing */
UBYTE flash_Write_Data(UDWORD WAddr, UDWORD WCnt, UBYTE * pData)
{
	// SW BUSY to High during FROM writing
    BUSY = 0;
	UDWORD				EndWAddr;						/* Writing end address					*/
	UDWORD				WPagCnt;						/* Writing page count					*/
	UDWORD				WByteCnt;						/* 1page writing byte count				*/
	UDWORD				StrPagNo;						/* Writing start page number			*/
	UDWORD				StrPagIdx;						/* Index in Writing start page			*/
	UBYTE				Ret;
	UBYTE				StsReg;

	/* Parameter check */
	EndWAddr = WAddr + WCnt;
	if (pData == NULL)
	{
		return FLASH_ERR;
	}
	if ((WCnt == 0) || (EndWAddr > FLASH_MEM_SIZE))
	{
		return FLASH_ERR;
	}

	/* Write-protection check */
	Ret = flash_Read_StsReg((UBYTE *)&StsReg);
	if (Ret != FLASH_OK)
	{
		return Ret;
	}

	/* Writing page calculation */
	StrPagNo  = WAddr / FLASH_WPAG_SIZE;				/* Writing start page number calculation*/
	WPagCnt   = (EndWAddr / FLASH_WPAG_SIZE) - StrPagNo + 1;
														/* Writing page count calculation		*/
	StrPagIdx = WAddr - (StrPagNo * FLASH_WPAG_SIZE);	/* Index in Writing start page calculation*/

	if (WPagCnt == 1) 									/* Not page stepping over writing ?		*/
	{
		WByteCnt = WCnt;
	}
	else
	{
		WByteCnt = FLASH_WPAG_SIZE - StrPagIdx;
	}

	/* Writing of amounts that are smaller than the first one page */
	if (StrPagIdx != 0)
	{
		Ret = flash_Write_Page(WAddr, (UWORD)WByteCnt, pData);
		if (Ret != FLASH_OK)
		{
			return Ret;
		}

		/* Next writing address calculation */
		WCnt  -= WByteCnt;
		WAddr += WByteCnt;
		pData += WByteCnt;
	}

	/* Writing of one page each */
	for (; WCnt > FLASH_WPAG_SIZE; WCnt -= FLASH_WPAG_SIZE)
	{
		Ret = flash_Write_Page(WAddr, (UWORD)FLASH_WPAG_SIZE, pData);
		if (Ret != FLASH_OK)
		{
			return Ret;
		}

		/* Next writing address calculation */
		WAddr += FLASH_WPAG_SIZE;
		pData += FLASH_WPAG_SIZE;
	}

	/* Writing of amounts that are smaller than the remaining one page */
	if (WCnt != 0)
	{
		Ret = flash_Write_Page(WAddr, (UWORD)WCnt, pData);
		if (Ret != FLASH_OK)
		{
			return Ret;
		}
	}
    BUSY = 1;
	return FLASH_OK;
}

/* SPI受信関数 */
static UBYTE flash_SPI_Rx(UWORD RxCnt, UBYTE * pData)
{
	UWORD				RxWait;								/* Receive waiting counter				*/

	RxWait = FLASH_SPI_RX_WAIT;								/* Wait 50ms							*/
	for (; RxCnt != 0; RxCnt--) 							/* Loop for receive byte				*/
	{
		/* Receive dummy data -> Transmission buffer register */
		FLASH_SPI_BUF = FLASH_DUMMY_DATA;					/* Receive dummy data setting			*/
		/* 受信完�?して�?るかチェ�?ク */
		while (FLASH_SPI_BF == FLASH_LOW)							/* Loop for receive completion			*/
		{
			RxWait--;
			if (RxWait == 0) 								/* Receive waiting time over			*/
			{
				return FLASH_ERR;
			}
			delay_us(FLASH_T_UART_WAIT);
		}
		*pData = (UBYTE)FLASH_SPI_BUF;						/* Receive data storage					*/
		pData++;

		if(FLASH_SPI_IF == FLASH_HI)								/* 割り込みフラグ消去					*/
		{
			FLASH_SPI_IF = FLASH_LOW;
		}
	}
	return FLASH_OK;
}

/* Data read processing */
UBYTE flash_Read_Data(UDWORD RAddr, UDWORD RCnt, UBYTE * pData)
{
	UBYTE				Ret;
	UWORD				Cnt;

	/* Parameter check */
	if ((RCnt == 0) || ((RAddr + RCnt) >  FLASH_MEM_SIZE))
	{
		return FLASH_ERR;
	}
    
	FLASH_SET_CS(FLASH_LOW);						/* CS "L"								*/
	delay_us(FLASH_T_CS_HOLD);
    
	/* Read command transmission */
	Ret = flash_Cmd_READ(RAddr);
	if (Ret != FLASH_OK)
	{
		return Ret;
	}

	delay_us(FLASH_T_R_ACCESS);

	/* Data reading */

	do
	{
		if(RCnt >= SHORT_SIZE)
		{
			Cnt = (UWORD)SHORT_SIZE;
		}
		else
		{
			Cnt = (UWORD)RCnt;
		}

		Ret = flash_SPI_Rx(Cnt, pData);				/* 1byte is read for speed-up 		*/
		if (Ret != FLASH_OK)
		{
			return Ret;
		}

		pData	+=	Cnt;
		RCnt	-=	Cnt;
	}while(RCnt != 0);

	delay_us(FLASH_T_CS_HOLD);
	FLASH_SET_CS(FLASH_HI);						/* CS "H"								*/

	return FLASH_OK;
}

/* Erase processing */
UBYTE flash_Erase(UDWORD EAddr, UBYTE Etype)
{
#ifdef FLASH_WEL_CHK
	UBYTE				StsReg;								/* Status buffer						*/
#endif	/* #ifdef FLASH_WEL_CHK */
	UBYTE				Ret;
    // SW BUSY to High during FROM writing
    //BUSY = 0;

	/* Writing enable(WEL set) */
	Ret = flash_Write_En();
	if (Ret != FLASH_OK)
	{
		return Ret;
	}

#ifdef FLASH_WEL_CHK
	/* Status register reading */
	Ret = flash_Read_StsReg((UBYTE *)&StsReg);
	if ((Ret != FLASH_OK) || ((StsReg & FLASH_REG_WEL) == 0x00))
	{
															/* Writing disable						*/
		return Ret;
	}
#endif	/* #ifdef FLASH_WEL_CHK */

	FLASH_SET_CS(FLASH_LOW);								/* CS "L"								*/
	delay_us(FLASH_T_CS_HOLD);

	/* Write command transmission */
	if(Etype==B_ERASE)
	{
		Ret = flash_Cmd_BE();								/*Bulk erase							*/
	}
	else
	{
		Ret = flash_Cmd_SE(EAddr);							/*Sector erase							*/
	}

	if (Ret != FLASH_OK)
	{
		return Ret;
	}

	delay_us(FLASH_T_CS_HOLD);
	FLASH_SET_CS(FLASH_HI);									/* CS "H"								*/

	/* Erase busy completion waiting */
	Ret = flash_Wait_Busy(FLASH_T_EBUSY_WAIT, FLASH_EBUSY_WAIT);
	if (Ret != FLASH_OK)
	{
		return Ret;
	}
    
    //BUSY = 1;

	return FLASH_OK;
}

/* Status register read processing */
UBYTE flash_Read_StsReg(UBYTE * pStsReg)
{
	UBYTE				Ret;

	FLASH_SET_CS(FLASH_LOW);							/* CS "L"								*/
	delay_us(FLASH_T_CS_HOLD);

	/* Status register read command transmission */
	Ret = flash_Cmd_RDSR();
	if (Ret != FLASH_OK)
	{
		return Ret;
	}

	delay_us(FLASH_T_R_ACCESS);

	/* Status register reading */
	Ret = flash_SPI_Rx(FLASH_STSREG_SIZE, pStsReg);
	if (Ret != FLASH_OK)
	{
		return Ret;
	}

	delay_us(FLASH_T_CS_HOLD);
	FLASH_SET_CS(FLASH_HI);							/* CS "H"								*/

	/* Fixed data division check */

	if ((*pStsReg & (UBYTE)0x60) != 0x00)
	{
		return FLASH_ERR;
	}

	return FLASH_OK;
}

/* Status register write processing */
UBYTE flash_Write_StsReg(UBYTE * pStsReg)
{
#ifdef FLASH_WEL_CHK
	UBYTE				StsReg;								/* Status buffer						*/
#endif	/* #ifdef FLASH_WEL_CHK */
	UBYTE				Ret;

	/* Write status fixed data setting */
	*pStsReg = *pStsReg & (UBYTE)0x9C;

	/* Writing enable(WEL set) */
	Ret = flash_Write_En();
	if (Ret != FLASH_OK)
	{
		return Ret;
	}

#ifdef FLASH_WEL_CHK
	/* Status register reading */
	Ret = flash_Read_StsReg((UBYTE *)&StsReg);
	if ((Ret != FLASH_OK) || ((StsReg & FLASH_REG_WEL) == 0x00))
	{													/* Writing disable						*/

		return Ret;
	}
#endif	/* #ifdef FLASH_WEL_CHK */

	FLASH_SET_CS(FLASH_LOW);							/* CS "L"								*/
	delay_us(FLASH_T_CS_HOLD);

	/* Status register write command transmission */
	Ret = flash_Cmd_WRSR();
	if (Ret != FLASH_OK)
	{
		return Ret;
	}

	/* Status register writing */
	Ret = flash_SPI_DataOut(FLASH_STSREG_SIZE, pStsReg);
	if (Ret != FLASH_OK)
	{
		return Ret;
	}

	delay_us(FLASH_T_CS_HOLD);
	FLASH_SET_CS(FLASH_HI);							/* CS "H"								*/

	/* Write busy completion waiting */
	Ret = flash_Wait_Busy(FLASH_T_WBUSY_WAIT, FLASH_WBUSY_WAIT);
	if (Ret != FLASH_OK)
	{
		return Ret;
	}

	return FLASH_OK;
}

/* Write-protection setting processing *
 *-----------------------------------------------------------------------------------------------
 * 				: write-protection setting data (WpSts).
 * 				:   WpSts=0 :	BP0=0	BP1=0,	BP2=0
 * 				:   WpSts=1 :	BP0=1	BP1=0,	BP2=0
 * 				:   WpSts=2 :	BP0=0	BP1=1,	BP2=0
 * 				:   WpSts=3 :	BP0=1	BP1=1,	BP2=0
 * 				:   WpSts=4 :	BP0=0	BP1=0,	BP2=1
 * 				:   WpSts=5 :	BP0=1	BP1=0,	BP2=1
 * 				:   WpSts=6 :	BP0=0	BP1=1,	BP2=1
 * 				:   WpSts=7 :	BP0=1	BP1=1,	BP2=1
 *---------------------------------------------------------------------------------------------*/
UBYTE flash_Write_Protect(UBYTE WpSts)
{
	UBYTE				StsReg;							/* Write status buffer					*/
	UBYTE				Ret;

	/* Parameter check */
	if (WpSts >  FLASH_WP_WHOLE_MEM)
	{
		return FLASH_ERR;
	}

	/* Write-protection setting value storage */
	StsReg = (WpSts << 2) & (~FLASH_REG_SRWD);			/* SRWD is fixed "0"					*/

	/* Status register writing */
	Ret = flash_Write_StsReg((UBYTE *)&StsReg);
	if (Ret != FLASH_OK)
	{
		return Ret;
	}

	/* Write-protection state setting */
//	gFlash_WP = WpSts;

	return FLASH_OK;
}

/*Deep sleep*/

UBYTE flash_Deep_sleep(void)
{
//	UBYTE				Ret;
//
//	FLASH_SET_CS(FLASH_LOW);								//* CS "L"
//	delay_us(FLASH_T_CS_HOLD);								//* 1us ウェイ�?
//
//	//* change to Deep sleep mode
//	Ret = flash_Cmd_DP();
//
//	delay_us(FLASH_T_CS_HOLD);								//* 1us ウェイ�?
//	FLASH_SET_CS(FLASH_HI);									//* CS "H"
//	return Ret;    
}

/*Release Deep sleep*/

UBYTE flash_Wake_up(void)
{
//	UBYTE				Ret;
//
//	FLASH_SET_CS(FLASH_LOW);								//* CS "L"
//	delay_us(FLASH_T_CS_HOLD);								//* 1us ウェイ�?
//
//	//* Release Deep sleep
//	Ret = flash_Cmd_RES();
//
//	delay_us(FLASH_T_CS_HOLD);								//* 1us ウェイ�?
//	FLASH_SET_CS(FLASH_HI);									//* CS "H"
//	return Ret;    
}
