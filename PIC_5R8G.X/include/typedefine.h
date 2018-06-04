/*typedefine.h*/

#ifndef __typedefine_h__
#define __typedefine_h__

typedef signed char SBYTE;
typedef unsigned char UBYTE;
typedef signed short SWORD;
typedef unsigned short UWORD;
typedef signed int SINT;
typedef unsigned int UINT;
typedef unsigned short long USLONG;
typedef signed long SDWORD;
typedef unsigned long UDWORD;

/* UWORD <-> UBYTE conversion */
typedef union{
	UDWORD	us;
	UBYTE	uc[4];
}EXCHG_LONG;	 //total 4byte

#define MaxOfMemory 40  //  TODO : Use Bank function then magnify buffer size;
const UBYTE FooterOfJPEG[] = {0xff, 0x1e};
const UBYTE FooterOfH264[] = {0x00, 0x00, 0x01, 0x1e};

#endif	//#ifndef __typedefine_h__