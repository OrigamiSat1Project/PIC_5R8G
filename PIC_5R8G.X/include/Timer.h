/*Timer.h*/

#include "typedefine.h"

#ifndef TIMER_H
#define	TIMER_H
/* Comment
 * =============================================================================
 * Timer processing
 * =============================================================================
 * Code
 * =============================================================================
 */
UINT timer_counter = 0;
void initInterrupt(void);
void interrupt incrementTimer(void);

#endif	/* Timer_H */