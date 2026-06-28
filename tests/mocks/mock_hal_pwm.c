#include "mock_hal_pwm.h"

DEFINE_FAKE_VALUE_FUNC1(hal_status_t, hal_pwm_timer_init, uint32_t);
DEFINE_FAKE_VALUE_FUNC1(hal_status_t, hal_pwm_channel_init, hal_pwm_channel_t);
DEFINE_FAKE_VALUE_FUNC2(hal_status_t, hal_pwm_enable, hal_pwm_channel_t, bool);
DEFINE_FAKE_VALUE_FUNC2(hal_status_t, hal_pwm_set_duty_cycle, hal_pwm_channel_t, uint8_t);
DEFINE_FAKE_VOID_FUNC1(hal_pwm_set_frequency, uint32_t);
