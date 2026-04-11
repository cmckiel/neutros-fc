#include "motor.h"
#include "pwm_motor.h"

bool motor_init()
{
  return pwm_motor_init();
}

bool motor_exec()
{
  return pwm_motor_exec();
}
