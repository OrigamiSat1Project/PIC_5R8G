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
UINT get_timer_counter(void);
void set_timer_counter_min(UINT);
UINT get_timer_counter_min(void);
void set_timer_counter_only_getUart(UINT);
UINT get_timer_counter_only_getUart(void);
void set_timer_counter_only_rest(UINT);
UINT get_timer_counter_only_rest(void);

#endif	/* Timer_H */