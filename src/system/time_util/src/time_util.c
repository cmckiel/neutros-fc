#include "time_util.h"

uint32_t time_util_calc_ticks_elapsed(uint32_t time_start, uint32_t time_now)
{
  return time_now - time_start;
}
