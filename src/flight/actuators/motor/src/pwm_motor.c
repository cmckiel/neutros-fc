#include "pwm_motor.h"
#include "hal/pwm.h"
#include "blackboard.h"
#include "c2_types.h"

static const c2_blackboard_data_t *c2_blackboard_data = NULL;

bool pwm_motor_init()
{
  c2_blackboard_data = (const c2_blackboard_data_t*)blackboard_get_subscriber_handle(C2_TOPIC_ID);

  hal_pwm_init(400);
  hal_pwm_enable(true);
  hal_pwm_set_duty_cycle(0);

  return true;
}

bool pwm_motor_exec()
{
  bool res = false;

  if (c2_blackboard_data)
  {
    hal_pwm_set_duty_cycle(c2_blackboard_data->public_data.commanded_motor_duty_cycle);
    res = true;
  }

  return res;
}
