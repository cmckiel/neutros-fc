#include "mock_hal_systick.h"

DEFINE_FAKE_VALUE_FUNC0(hal_status_t, hal_systick_init);
DEFINE_FAKE_VALUE_FUNC0(uint32_t, hal_get_tick);
DEFINE_FAKE_VOID_FUNC1(hal_delay_ms, uint32_t);
DEFINE_FAKE_VALUE_FUNC3(hal_timer_handle_t, hal_systick_timer_register, uint32_t, hal_timer_type_t, hal_timer_callback_t);
DEFINE_FAKE_VOID_FUNC1(hal_systick_timer_deregister, hal_timer_handle_t);
