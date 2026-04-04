#include "telemetry.h"
#include "config.h"
#include "blackboard.h"
#include "c2_types.h"
#include "hal/uart.h"
#include "imu_types.h"

#include <string.h>
#include <stdio.h>

static hal_uart_t uart_channel;

static const c2_blackboard_data_t *c2_blackboard_data;
static const imu_blackboard_data_t *imu_blackboard_data;

static bool telemetry_is_initialized = false;

bool telemetry_init()
{
  uart_channel = system_config.telemetry_uart_channel;

  c2_blackboard_data = (const c2_blackboard_data_t*)blackboard_get_subscriber_handle(C2_TOPIC_ID);
  imu_blackboard_data = (const imu_blackboard_data_t*)blackboard_get_subscriber_handle(IMU_TOPIC_ID);

  telemetry_is_initialized = true;

  hal_status_t status = hal_uart_init(uart_channel);

  return (status == HAL_STATUS_OK) ? true : false;
}

bool telemetry_exec()
{
  bool res = false;

  if (telemetry_is_initialized)
  {
    // const size_t LEN = 10;
    // uint8_t data[LEN];
    size_t bytes_written = 0;

    // memcpy(data, c2_blackboard_data->public_data.received_data_raw, sizeof(data));

    // hal_status_t status = hal_uart_write(uart_channel, data, sizeof(data), &bytes_written);
    // bytes_written = 0;

    uint8_t imu_message[100];
    float x = imu_blackboard_data->public_data.gyroscope.x_angular_velocity_dps;
    float y = imu_blackboard_data->public_data.gyroscope.y_angular_velocity_dps;
    float z = imu_blackboard_data->public_data.gyroscope.z_angular_velocity_dps;

    snprintf((char*)imu_message, sizeof(imu_message), "\033[2J\033[H imu_data:{gx_dps: %.2f, gy_dps: %.2f, gz_dps: %.2f}", x, y, z);
    hal_status_t status = hal_uart_write(uart_channel, imu_message, sizeof(imu_message), &bytes_written);

    res = (status == HAL_STATUS_OK) ? true : false;
  }

  return res;
}
