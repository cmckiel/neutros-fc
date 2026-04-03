#include "hal/uart.h"

typedef struct {
  hal_uart_t c2_uart_channel;
  hal_uart_t telemetry_uart_channel;
} system_config_t;

extern system_config_t system_config;
