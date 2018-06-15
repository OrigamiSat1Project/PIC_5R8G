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

void initInterrupt(void);
void interrupt incrementTimer(void);
void set_timer_counter(UINT);
void set_timer_counter_min(UINT);
UINT get_timer_counter(void);
UINT get_timer_counter_min(void);

#endif	/* Timer_H */