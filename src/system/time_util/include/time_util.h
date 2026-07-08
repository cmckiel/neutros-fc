#ifndef _TIME_UTIL_H
#define _TIME_UTIL_H

#include <stdint.h>
#include <stdbool.h>

uint32_t time_util_calc_ticks_elapsed(uint32_t time_start, uint32_t time_now);

bool time_util_timed_out(uint32_t time_start, uint32_t time_now, uint32_t timeout);

#endif /* _TIME_UTIL_H */
