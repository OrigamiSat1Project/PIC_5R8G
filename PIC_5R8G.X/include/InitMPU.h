/* 
 * File:   InitMPU.h
 * Author: LP
 *
 * Created on 2015/06/30, 18:52
 */

#ifndef INITMPU_H
#define	INITMPU_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef __typedefine_h__
#include "typedefine.h"
#endif

#define WDT_CLK		(RB6)

void init_mpu(void);

#ifdef	__cplusplus
}
#endif

#endif	/* INITMPU_H */

