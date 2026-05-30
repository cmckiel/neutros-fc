#include "pwm_motor.h"
#include "hal/pwm.h"
#include "blackboard.h"
#include "c2_types.h"

static const c2_blackboard_data_t *c2_blackboard_data = NULL;

static uint8_t m1_dc = 0;
static uint8_t m2_dc = 0;
static uint8_t m3_dc = 0;
static uint8_t m4_dc = 0;

bool pwm_motor_init()
{
  c2_blackboard_data = (const c2_blackboard_data_t*)blackboard_get_subscriber_handle(C2_TOPIC_ID);

  hal_pwm_timer_init(400);

  hal_pwm_channel_init(HAL_PWM_CH1);
  hal_pwm_channel_init(HAL_PWM_CH2);
  hal_pwm_channel_init(HAL_PWM_CH3);
  hal_pwm_channel_init(HAL_PWM_CH4);

  hal_pwm_enable(HAL_PWM_CH1, true);
  hal_pwm_enable(HAL_PWM_CH2, true);
  hal_pwm_enable(HAL_PWM_CH3, true);
  hal_pwm_enable(HAL_PWM_CH4, true);

  hal_pwm_set_duty_cycle(HAL_PWM_CH1, 0);
  hal_pwm_set_duty_cycle(HAL_PWM_CH2, 0);
  hal_pwm_set_duty_cycle(HAL_PWM_CH3, 0);
  hal_pwm_set_duty_cycle(HAL_PWM_CH4, 0);

  return true;
}

bool pwm_motor_exec()
{
  bool res = false;

  if (c2_blackboard_data)
  {

    uint8_t m1_dc_request = c2_blackboard_data->public_data.commanded_motor_1_duty_cycle;
    uint8_t m2_dc_request = c2_blackboard_data->public_data.commanded_motor_2_duty_cycle;
    uint8_t m3_dc_request = c2_blackboard_data->public_data.commanded_motor_3_duty_cycle;
    uint8_t m4_dc_request = c2_blackboard_data->public_data.commanded_motor_4_duty_cycle;

    if (m1_dc != m1_dc_request)
    {
      hal_pwm_set_duty_cycle(HAL_PWM_CH1, m1_dc_request);
      m1_dc = m1_dc_request;
    }

    if (m2_dc != m2_dc_request)
    {
      hal_pwm_set_duty_cycle(HAL_PWM_CH2, m2_dc_request);
      m2_dc = m2_dc_request;
    }

    if (m3_dc != m3_dc_request)
    {
      hal_pwm_set_duty_cycle(HAL_PWM_CH3, m3_dc_request);
      m3_dc = m3_dc_request;
    }

    if (m4_dc != m4_dc_request)
    {
      hal_pwm_set_duty_cycle(HAL_PWM_CH4, m4_dc_request);
      m4_dc = m4_dc_request;
    }

    // hal_pwm_set_duty_cycle(HAL_PWM_CH1, c2_blackboard_data->public_data.commanded_motor_1_duty_cycle);
    // hal_pwm_set_duty_cycle(HAL_PWM_CH2, c2_blackboard_data->public_data.commanded_motor_2_duty_cycle);
    // hal_pwm_set_duty_cycle(HAL_PWM_CH3, c2_blackboard_data->public_data.commanded_motor_3_duty_cycle);
    // hal_pwm_set_duty_cycle(HAL_PWM_CH4, c2_blackboard_data->public_data.commanded_motor_4_duty_cycle);

    res = true;
  }

  return res;
}
