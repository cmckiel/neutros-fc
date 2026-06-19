#include "mock_hal_gpio.h"

DEFINE_FAKE_VALUE_FUNC0(hal_status_t, hal_gpio_init);
DEFINE_FAKE_VALUE_FUNC0(hal_status_t, hal_gpio_toggle_led);
