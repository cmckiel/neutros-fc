#include "c2.h"
#include "c2_types.h"
#include "hal/uart.h"
#include "config.h"
#include "blackboard.h"

#include <string.h>

static hal_uart_t uart_channel;
static c2_blackboard_data_t *c2_blackboard_data;
static bool c2_initialized = false;

static int current_active_channel = 1; // [1, 4] inclusive

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
    uint8_t data = 0;
    size_t bytes_read = 0;

    hal_status_t status = hal_uart_read(uart_channel, &data, 1, &bytes_read);

    // Select active channel
    if (data == '1')
    {
        current_active_channel = 1;
    }
    else if (data == '2')
    {
        current_active_channel = 2;
    }
    else if (data == '3')
    {
        current_active_channel = 3;
    }
    else if (data == '4')
    {
        current_active_channel = 4;
    }
    else if (data == '5')
    {
        current_active_channel = 5;
    }

    // Store selected channel's current duty cycle.
    uint8_t duty_cycle = 0;
    if (current_active_channel == 1)
    {
        duty_cycle = c2_blackboard_data->public_data.commanded_motor_1_duty_cycle;
    }
    else if (current_active_channel == 2)
    {
        duty_cycle = c2_blackboard_data->public_data.commanded_motor_2_duty_cycle;
    }
    else if (current_active_channel == 3)
    {
        duty_cycle = c2_blackboard_data->public_data.commanded_motor_3_duty_cycle;
    }
    else if (current_active_channel == 4)
    {
        duty_cycle = c2_blackboard_data->public_data.commanded_motor_4_duty_cycle;
    }
    else if (current_active_channel == 5)
    {
        duty_cycle = c2_blackboard_data->public_data.commanded_motor_1_duty_cycle;
    }

    // Manipulate that duty cycle based off further command
    if (data == 'u')
    {
        duty_cycle += 5;
        if (duty_cycle > 80)
        {
            duty_cycle = 80;
        }
        else if (duty_cycle < 40)
        {
            duty_cycle = 40;
        }
    }
    else if (data == 'd')
    {
        if (duty_cycle >= 45)
        {
            duty_cycle -= 5;
        }
        else
        {
            duty_cycle = 40;
        }
    }
    else if (data == '0')
    {
        duty_cycle = 0;
    }
    else if (data == '8')
    {
        duty_cycle = 80;
    }
    else if (data == '7')
    {
        duty_cycle = 40;
    }

    // Publish new duty cycle value to the blackboard.
    if (current_active_channel == 1)
    {
        c2_blackboard_data->public_data.commanded_motor_1_duty_cycle = duty_cycle;
    }
    else if (current_active_channel == 2)
    {
        c2_blackboard_data->public_data.commanded_motor_2_duty_cycle = duty_cycle;
    }
    else if (current_active_channel == 3)
    {
        c2_blackboard_data->public_data.commanded_motor_3_duty_cycle = duty_cycle;
    }
    else if (current_active_channel == 4)
    {
        c2_blackboard_data->public_data.commanded_motor_4_duty_cycle = duty_cycle;
    }
    else if (current_active_channel == 5)
    {
        c2_blackboard_data->public_data.commanded_motor_1_duty_cycle = duty_cycle;
        c2_blackboard_data->public_data.commanded_motor_2_duty_cycle = duty_cycle;
        c2_blackboard_data->public_data.commanded_motor_3_duty_cycle = duty_cycle;
        c2_blackboard_data->public_data.commanded_motor_4_duty_cycle = duty_cycle;
    }

    res = (status == HAL_STATUS_OK) ? true : false;
  }

  return res;
}
