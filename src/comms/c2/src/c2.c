#include "c2.h"
#include "c2_types.h"
#include "hal/uart.h"
#include "config.h"
#include "blackboard.h"

#include <string.h>

static hal_uart_t uart_channel;
static c2_blackboard_data_t *c2_blackboard_data;
static bool c2_initialized = false;

bool c2_init()
{
  // Set local state vairables and acquire handle to publish c2's topic.
  uart_channel = system_config.c2_uart_channel;
  c2_blackboard_data = (c2_blackboard_data_t*)blackboard_get_publisher_handle(C2_TOPIC_ID);
  c2_initialized = true;

  // Init the uart channel c2 was assigned.
  hal_status_t status = hal_uart_init(uart_channel);

  return (status == HAL_STATUS_OK) ? true : false;
}

bool c2_exec()
{
  bool res = false;

  if (c2_initialized)
  {
    uint8_t data[10] = {0};
    size_t bytes_read = 0;

    hal_status_t status = hal_uart_read(uart_channel, data, sizeof(data), &bytes_read);

    size_t min_data_size = (sizeof(c2_blackboard_data->public_data.received_data_raw) > sizeof(data)) ?
      sizeof(data) : sizeof(c2_blackboard_data->public_data.received_data_raw);

    memcpy(c2_blackboard_data->public_data.received_data_raw, data, min_data_size);

    res = (status == HAL_STATUS_OK) ? true : false;
  }

  return res;
}
