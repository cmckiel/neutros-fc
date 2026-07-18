#include "imu_mpu6050.h"
#include "mpu6050_init.h"
#include "mpu6050.h"
#include "hal/i2c.h"
#include "hal/systick.h"
#include "time_util.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

static float gx_dps = 0;
static float gy_dps = 0;
static float gz_dps = 0;

static int16_t xg_offset = 0;
static int16_t yg_offset = 0;
static int16_t zg_offset = 0;

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

static bool reset_i2c_transaction(hal_i2c_txn_t *txn)
{
	if (!txn)
	{
		return false;
	}

	// Reset control fields
	// txn->target_addr = MPU_6050_ADDR;
	// txn->i2c_op = HAL_I2C_OP_WRITE;
	// txn->expected_bytes_to_tx = 0;
	// txn->expected_bytes_to_rx = 0;

	// Reset buffers
	memset(txn->tx_data, 0, sizeof(txn->tx_data));
	memset(txn->rx_data, 0, sizeof(txn->rx_data));

	// Reset result fields
	txn->processing_state = HAL_I2C_TXN_STATE_CREATED;
	txn->transaction_result = HAL_I2C_TXN_RESULT_NONE;
	txn->actual_bytes_received = 0;
	txn->actual_bytes_transmitted = 0;

	return true;
}

static bool grade_self_test(float percent_diff_from_factory)
{
	// self test response must be within +/- 14% of factory trim
	return -14.0 < percent_diff_from_factory && percent_diff_from_factory < 14.0;
}

bool imu_mpu6050_init()
{
	if (!mpu6050_whoami())
	{
		printf("whoami failed!\r\n");
		return false;
	}

	if (!mpu6050_set_clock_source_gyro_x())
	{
		printf("set x-gyro clock failed\r\n");
		return false;
	}

	float xg_percent_diff_from_factory = 0.0;
	float yg_percent_diff_from_factory = 0.0;
	float zg_percent_diff_from_factory = 0.0;

	float xa_percent_diff_from_factory = 0.0;
	float ya_percent_diff_from_factory = 0.0;
	float za_percent_diff_from_factory = 0.0;

	if (!mpu6050_perform_self_test(&xg_percent_diff_from_factory, &yg_percent_diff_from_factory, &zg_percent_diff_from_factory,
																 &xa_percent_diff_from_factory, &ya_percent_diff_from_factory, &za_percent_diff_from_factory))
	{
		printf("failed to perform self test\r\n");
		return false;
	}

	printf("\r\n");
	printf("Self test resutls:\r\n");
	printf("gyro:\r\n");
	printf("xg: %0.4f %s\r\n", xg_percent_diff_from_factory, grade_self_test(xg_percent_diff_from_factory) ? "PASS" : "FAIL");
	printf("yg: %0.4f %s\r\n", yg_percent_diff_from_factory, grade_self_test(yg_percent_diff_from_factory) ? "PASS" : "FAIL");
	printf("zg: %0.4f %s\r\n", zg_percent_diff_from_factory, grade_self_test(zg_percent_diff_from_factory) ? "PASS" : "FAIL");
	printf("accel:\r\n");
	printf("xa: %0.4f %s\r\n", xa_percent_diff_from_factory, grade_self_test(xa_percent_diff_from_factory) ? "PASS" : "FAIL");
	printf("ya: %0.4f %s\r\n", ya_percent_diff_from_factory, grade_self_test(ya_percent_diff_from_factory) ? "PASS" : "FAIL");
	printf("za: %0.4f %s\r\n", za_percent_diff_from_factory, grade_self_test(za_percent_diff_from_factory) ? "PASS" : "FAIL");
	printf("\r\n");

	if (!grade_self_test(xg_percent_diff_from_factory) || !grade_self_test(yg_percent_diff_from_factory) || !grade_self_test(zg_percent_diff_from_factory) ||
			!grade_self_test(xa_percent_diff_from_factory) || !grade_self_test(ya_percent_diff_from_factory) || !grade_self_test(za_percent_diff_from_factory))
	{
		return false;
	}

	if (!mpu6050_set_fs_sel_gyro(MPU6050_FS_SEL_GYRO_2000_DPS))
	{
		// return false;
	}

	if (!mpu6050_calc_gyro_offsets(&xg_offset, &yg_offset, &zg_offset))
	{
		// return false;
	}

	printf("Offsets:\r\n");
	printf("xg_offset: %d\r\n", xg_offset);
	printf("yg_offset: %d\r\n", yg_offset);
	printf("zg_offset: %d\r\n", zg_offset);
	printf("\r\n");

	return true;
}



// @TODO why is this not reading gyro consistently??
/**
 * @brief gets the angular acceleration from the imu
 *
 * @impl LLR_IMU_002
 */
bool imu_mpu6050_get_angular_acceleration(float *gx_dps_ptr, float *gy_dps_ptr, float *gz_dps_ptr)
{
	bool res = false;

	if (imu_read_gyro.processing_state == HAL_I2C_TXN_STATE_COMPLETED)
	{
		// Update my display data.
		int16_t gx = (imu_read_gyro.rx_data[0] << 8) | imu_read_gyro.rx_data[1];
		int16_t gy = (imu_read_gyro.rx_data[2] << 8) | imu_read_gyro.rx_data[3];
		int16_t gz = (imu_read_gyro.rx_data[4] << 8) | imu_read_gyro.rx_data[5];

		// gx_dps = (float)(gx - xg_offset) / 16.4f;
		// gy_dps = (float)(gy - yg_offset) / 16.4f;
		// gz_dps = (float)(gz - zg_offset) / 16.4f;

		gx_dps = (float)(gx) / 16.4f;
		gy_dps = (float)(gy) / 16.4f;
		gz_dps = (float)(gz) / 16.4f;

		if (gx_dps_ptr && gy_dps_ptr && gz_dps_ptr)
		{
			*gx_dps_ptr = gx_dps;
			*gy_dps_ptr = gy_dps;
			*gz_dps_ptr = gz_dps;
		}

		reset_i2c_transaction(&imu_read_gyro);

		res = true;
	}
	else if (imu_read_gyro.processing_state == HAL_I2C_TXN_STATE_CREATED)
	{
		hal_i2c_submit_transaction(&imu_read_gyro);
	}

	return res;
}
