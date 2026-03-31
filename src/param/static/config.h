#include "hal/uart.h"
#include "scheduler.h"

typedef hal_uart_t c2_uart_channel_t;

typedef struct {
  c2_uart_channel_t c2_uart_channel;
} system_config_t;

extern system_config_t system_config;
extern schedule_t schedule;
