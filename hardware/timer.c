#include "timer.h"
#include "address_map.h"

#define TIMER_STATUS    ((volatile int *)(TIMER_BASE))
#define TIMER_CONTROL   ((volatile int *)(TIMER_BASE + 4))
#define TIMER_PERIODL   ((volatile int *)(TIMER_BASE + 8))
#define TIMER_PERIODH   ((volatile int *)(TIMER_BASE + 12))

#define SYSTEM_CLOCK_FREQ 100000000