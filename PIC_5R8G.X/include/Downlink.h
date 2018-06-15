/*Downlink.h*/

#ifndef DOWNLINK_H
#define	DOWNLINK_H

#include "typedefine.h"

void Downlink(UDWORD, UDWORD, UBYTE);
void Downlink_clock(UDWORD, UDWORD, UBYTE, UINT);
void set_rest_time(UINT);
UINT get_rest_time(void);

#endif	/* DOWNLINK_H */