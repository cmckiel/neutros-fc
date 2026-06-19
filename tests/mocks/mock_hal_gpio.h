#pragma once

#include "fff.h"
#include "hal/gpio.h"

DECLARE_FAKE_VALUE_FUNC0(hal_status_t, hal_gpio_init);
DECLARE_FAKE_VALUE_FUNC0(hal_status_t, hal_gpio_toggle_led);
