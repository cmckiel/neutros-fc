#include "i2c_servicer.h"
#include "hal/i2c.h"

bool i2c_servicer_init()
{
  hal_status_t status = hal_i2c_init();
  return (status == HAL_STATUS_OK) ? true : false;
}

bool i2c_servicer_exec()
{
  hal_status_t status = hal_i2c_transaction_servicer();
  return (status == HAL_STATUS_OK) ? true : false;
}
