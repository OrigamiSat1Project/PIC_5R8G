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

#define MaxOfMemory 96

#endif	//#ifndef __typedefine_h__
