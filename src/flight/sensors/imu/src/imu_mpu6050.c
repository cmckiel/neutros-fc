#include "imu_mpu6050.h"
#include "hal/i2c.h"

#include <stdbool.h>

#define MPU_6050_ADDR       0x68
#define MPU_PWR_MGMT_1_REG  0x6B
#define MPU_GYRO_XOUT_H_REG 0x43
#define MPU_GYRO_WAKE       0x01
#define MPU_GYRO_SLEEP      0x40

static float gx_dps = 0;
static float gy_dps = 0;
static float gz_dps = 0;

static hal_i2c_txn_t imu_read_pwr_mode = {
	// Immutable once submitted.
	.target_addr = MPU_6050_ADDR,
	.i2c_op = HAL_I2C_OP_WRITE_READ,
	.tx_data = { MPU_PWR_MGMT_1_REG },
	.expected_bytes_to_tx = 1,
	.expected_bytes_to_rx = 1,

	// Poll to determine completion status.
	.processing_state = HAL_I2C_TXN_STATE_CREATED,

	// Post transaction completion results.
	.transaction_result = HAL_I2C_TXN_RESULT_NONE,
	.actual_bytes_received = 0,
	.actual_bytes_transmitted = 0,
	.rx_data = {0},
};

static hal_i2c_txn_t imu_wake_gyro = {
	// Immutable once submitted.
	.target_addr = MPU_6050_ADDR,
	.i2c_op = HAL_I2C_OP_WRITE,
	.tx_data = { MPU_PWR_MGMT_1_REG, MPU_GYRO_WAKE},
	.expected_bytes_to_tx = 2,
	.expected_bytes_to_rx = 0,

	// Poll to determine completion status.
	.processing_state = HAL_I2C_TXN_STATE_CREATED,

	// Post transaction completion results.
	.transaction_result = HAL_I2C_TXN_RESULT_NONE,
	.actual_bytes_received = 0,
	.actual_bytes_transmitted = 0,
	.rx_data = {0},
};

static hal_i2c_txn_t imu_read_gyro = {
	// Immutable once submitted.
	.target_addr = MPU_6050_ADDR,
	.i2c_op = HAL_I2C_OP_WRITE_READ,
	.tx_data = { MPU_GYRO_XOUT_H_REG },
	.expected_bytes_to_tx = 1,
	.expected_bytes_to_rx = 6, // Burst read starting at XOUT_H

	// Poll to determine completion status.
	.processing_state = HAL_I2C_TXN_STATE_CREATED,

	// Post transaction completion results.
	.transaction_result = HAL_I2C_TXN_RESULT_NONE,
	.actual_bytes_received = 0,
	.actual_bytes_transmitted = 0,
	.rx_data = {0},
};

static hal_i2c_txn_t *current_transaction = &imu_read_pwr_mode;

void reset_i2c_transaction(hal_i2c_txn_t *txn)
{
	txn->processing_state = HAL_I2C_TXN_STATE_CREATED;
	txn->transaction_result = HAL_I2C_TXN_RESULT_NONE;
	txn->actual_bytes_received = 0;
	txn->actual_bytes_transmitted = 0;
	txn->rx_data[0] = 0; // Cheating for now. @todo memset()
	txn->rx_data[1] = 0; // Cheating for now. @todo memset()
	txn->rx_data[2] = 0; // Cheating for now. @todo memset()
	txn->rx_data[3] = 0; // Cheating for now. @todo memset()
	txn->rx_data[4] = 0; // Cheating for now. @todo memset()
	txn->rx_data[5] = 0; // Cheating for now. @todo memset()
}

bool imu_mpu6050_init()
{
	return true;
}

bool imu_mpu6050_get_angular_acceleration(float *gx_dps_ptr, float *gy_dps_ptr, float *gz_dps_ptr)
{
	bool res = false;

	// If we have a new transaction, submit it.
	if (current_transaction->processing_state == HAL_I2C_TXN_STATE_CREATED)
	{
		hal_i2c_submit_transaction(current_transaction);
	}
	// If the transaction is complete and some basic expectations check out, process the data and reset.
	else if (current_transaction->processing_state == HAL_I2C_TXN_STATE_COMPLETED)
	{
		// Determine next transaction.
		if (current_transaction == &imu_read_pwr_mode)
		{
			// Just received the results of sleep mode.
			uint8_t pwr_mode = imu_read_pwr_mode.rx_data[0];
			if (pwr_mode == MPU_GYRO_SLEEP)
			{
				// If it is sleeping we need to wake it.
				current_transaction = &imu_wake_gyro;
			}
			else if (pwr_mode == MPU_GYRO_WAKE)
			{
				current_transaction = &imu_read_gyro;
			}
			// Reset our transaction now that it is through.
			reset_i2c_transaction(&imu_read_pwr_mode);
		}
		else if (current_transaction == &imu_wake_gyro)
		{
			// Read back what we wrote.
			current_transaction = &imu_read_pwr_mode;
			reset_i2c_transaction(&imu_wake_gyro);
		}
		else if (current_transaction == &imu_read_gyro)
		{
			if (imu_read_gyro.transaction_result == HAL_I2C_TXN_RESULT_SUCCESS)
			{
				// Update my display data.
				int16_t gx = (imu_read_gyro.rx_data[0] << 8) | imu_read_gyro.rx_data[1];
				int16_t gy = (imu_read_gyro.rx_data[2] << 8) | imu_read_gyro.rx_data[3];
				int16_t gz = (imu_read_gyro.rx_data[4] << 8) | imu_read_gyro.rx_data[5];

				gx_dps = (float)gx / 131.0f;
				gy_dps = (float)gy / 131.0f;
				gz_dps = (float)gz / 131.0f;

				res = true;
			}
			else
			{
				current_transaction = &imu_read_pwr_mode;
			}

			reset_i2c_transaction(&imu_read_gyro);
		}
		else
		{
			// We don't know what transaction that was, reset back to read pwr.
			reset_i2c_transaction(&imu_read_pwr_mode);
			current_transaction = &imu_read_pwr_mode;
		}
	}

	if (gx_dps_ptr && gy_dps_ptr && gz_dps_ptr)
	{
		*gx_dps_ptr = gx_dps;
		*gy_dps_ptr = gy_dps;
		*gz_dps_ptr = gz_dps;
	}

	return res;
}
