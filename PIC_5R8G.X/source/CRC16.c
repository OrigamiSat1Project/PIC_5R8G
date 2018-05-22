#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "typedefine.h"

UWORD crc16(UWORD crc, UBYTE *ptr, UINT len)
{
#define CRC16POLY 0xa001
	UINT i, j;
	crc = ~crc;
	for (i = 0; i < len; i++) {
		crc ^= ptr[i];
		for (j = 0; j < 8; j++) {
			if (crc & 1) {
				crc = (crc >> 1) ^ CRC16POLY;
			}
			else {
				crc >>= 1;
			}
		}
	}
	return ~crc;
}

UWORD Identify_CRC16(UBYTE *text)
{
	UBYTE *p = (UBYTE *)text;
	UINT n = strlen(text);
	UWORD crc = crc16(0, p, n);
    return crc;
}