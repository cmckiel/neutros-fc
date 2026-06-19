#include "mock_hal_uart.h"

DEFINE_FAKE_VALUE_FUNC1(int, __io_putchar, int);
DEFINE_FAKE_VALUE_FUNC1(hal_status_t, hal_uart_init, hal_uart_t);
DEFINE_FAKE_VALUE_FUNC1(hal_status_t, hal_uart_deinit, hal_uart_t);
DEFINE_FAKE_VALUE_FUNC4(hal_status_t, hal_uart_read, hal_uart_t, uint8_t *, size_t, size_t *);
DEFINE_FAKE_VALUE_FUNC4(hal_status_t, hal_uart_write, hal_uart_t, const uint8_t *, size_t, size_t *);
