#pragma once

#include "fff.h"
#include "hal/i2c.h"

DECLARE_FAKE_VALUE_FUNC0(hal_status_t, hal_i2c_init);
DECLARE_FAKE_VALUE_FUNC1(hal_status_t, hal_i2c_submit_transaction, hal_i2c_txn_t *);
DECLARE_FAKE_VALUE_FUNC0(hal_status_t, hal_i2c_transaction_servicer);
