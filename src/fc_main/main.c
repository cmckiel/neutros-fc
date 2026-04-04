#include "hal/hal_system.h"
#include "scheduler.h"

int main()
{
	// Init System.
	hal_system_init();

  scheduler_init();
  scheduler_exec(); // no return

  return 1;
}
