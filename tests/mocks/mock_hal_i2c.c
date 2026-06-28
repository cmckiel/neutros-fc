#include "mock_hal_i2c.h"

DEFINE_FAKE_VALUE_FUNC0(hal_status_t, hal_i2c_init);
DEFINE_FAKE_VALUE_FUNC1(hal_status_t, hal_i2c_submit_transaction, hal_i2c_txn_t *);
DEFINE_FAKE_VALUE_FUNC0(hal_status_t, hal_i2c_transaction_servicer);
