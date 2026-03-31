#include "c2.h"
#include "hal/uart.h"
#include "config.h"

static bool stream_reader(uint8_t *data, size_t len, size_t *bytes_read)
{
  hal_status_t status = hal_uart_read(system_config.c2_uart_channel, data, len, bytes_read);
  return (status == HAL_STATUS_OK) ? true : false;
}

static bool stream_writer(uint8_t *data, size_t len, size_t *bytes_written)
{
  hal_status_t status = hal_uart_write(system_config.c2_uart_channel, data, len, bytes_written);
  return (status == HAL_STATUS_OK) ? true : false;
}

bool c2_init()
{
  hal_status_t status = hal_uart_init(system_config.c2_uart_channel);
  return (status == HAL_STATUS_OK) ? true : false;
}

bool c2_exec()
{
  const size_t LEN = 10;
  uint8_t data[LEN] = {0};
  size_t bytes_read = 0;

  bool res = stream_reader(data, sizeof(data), bytes_read);

  return res;
}
