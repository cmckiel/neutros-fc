#include "imu_mpu6050.h"
#include "hal/i2c.h"
#include "hal/systick.h"
#include "time_util.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// Device Address
#define MPU_6050_ADDR       0x68

// Device Registers
#define MPU_PWR_MGMT_1_REG   0x6B
#define MPU_ACCEL_XOUT_H_REG 0x3B
#define MPU_GYRO_XOUT_H_REG  0x43
#define MPU_WHOAMI_REG       0x75
#define MPU_SELF_TEST_X_REG  0x0D
#define MPU_GYRO_CONFIG_REG  0x1B
#define MPU_ACCEL_CONFIG_REG 0x1C

// Register Values
#define MPU_GYRO_WAKE       0x01
#define MPU_GYRO_SLEEP      0x40

typedef struct {
	uint8_t xg_test;
	uint8_t yg_test;
	uint8_t zg_test;

	uint8_t xa_test;
	uint8_t ya_test;
	uint8_t za_test;
} self_test_values_t;

typedef struct {
	uint16_t xg;
	uint16_t yg;
	uint16_t zg;

	uint16_t xa;
	uint16_t ya;
	uint16_t za;
} imu_values_t;

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

static hal_i2c_txn_t i2c_txn = {0};

static hal_i2c_txn_t *current_transaction = &imu_read_pwr_mode;

static float factory_trim_xg = 0;
static float factory_trim_yg = 0;
static float factory_trim_zg = 0;

static float factory_trim_xa = 0;
static float factory_trim_ya = 0;
static float factory_trim_za = 0;

static float calc_factory_trim_accel_x(float xa_test)
{
	if (xa_test == 0)
	{
		return 0.0;
	}

	float a = pow(2, 5) - 2.0;

	float b = 0.92 / 0.34;
	float c = (xa_test - 1.0) / a;

	float d = pow(b, c);

	return 4096.0 * 0.34 * d;
}

static float calc_factory_trim_accel_y(float ya_test)
{
	if (ya_test == 0)
	{
		return 0.0;
	}

	float a = pow(2, 5) - 2.0;

	float b = 0.92 / 0.34;
	float c = (ya_test - 1.0) / a;

	float d = pow(b, c);

	return 4096.0 * 0.34 * d;
}

static float calc_factory_trim_accel_z(float za_test)
{
	if (za_test == 0)
	{
		return 0.0;
	}

	float a = pow(2, 5) - 2.0;

	float b = 0.92 / 0.34;
	float c = (za_test - 1.0) / a;

	float d = pow(b, c);

	return 4096.0 * 0.34 * d;
}

static float calc_factory_trim_gyro_x(float xg_test)
{
	if (xg_test == 0)
	{
		return 0.0;
	}

	return 25.0 * 131.0 * pow(1.046, (double)xg_test - 1.0);
}

static float calc_factory_trim_gyro_y(float yg_test)
{
	if (yg_test == 0)
	{
		return 0.0;
	}

	return -25.0 * 131.0 * pow(1.046, (double)yg_test - 1.0);
}

static float calc_factory_trim_gyro_z(float zg_test)
{
	if (zg_test == 0)
	{
		return 0.0;
	}

	return 25.0 * 131.0 * pow(1.046, (double)zg_test - 1.0);
}

static float calc_self_test_response(float gyro_out_with_self_test_enabled, float gyro_out_with_self_test_disabled)
{
	return gyro_out_with_self_test_enabled - gyro_out_with_self_test_disabled;
}

static float calc_change_percent_from_factory_trim(float self_test_response, float factory_trim)
{
	if (factory_trim == 0)
	{
		return 0;
	}

	return (self_test_response - factory_trim) / factory_trim;
}

static void unpack_self_test(uint8_t *packed_bytes, uint32_t packed_bytes_size, self_test_values_t *self_test_values)
{
	if (packed_bytes && self_test_values && packed_bytes_size >= 4)
	{
		// gyro values just need to ignore accel portion
		self_test_values->xg_test = packed_bytes[0] & 0x1F;
		self_test_values->yg_test = packed_bytes[1] & 0x1F;
		self_test_values->zg_test = packed_bytes[2] & 0x1F;

		// need to reconstruct the accel values from two places
		self_test_values->xa_test = ((packed_bytes[0] & 0xE0) >> 3) | ((packed_bytes[3] & 0x30) >> 4);
		self_test_values->ya_test = ((packed_bytes[1] & 0xE0) >> 3) | ((packed_bytes[3] & 0x0A) >> 2);
		self_test_values->za_test = ((packed_bytes[2] & 0xE0) >> 3) | ((packed_bytes[3] & 0x03) >> 0);
	}
}

static void unpack_imu_values(uint8_t *packed_bytes, uint32_t packed_bytes_size, imu_values_t *imu_values)
{
	if (packed_bytes && imu_values && packed_bytes_size >= 14)
	{
		imu_values->xa = (packed_bytes[0] << 8) | packed_bytes[1];
		imu_values->ya = (packed_bytes[2] << 8) | packed_bytes[3];
		imu_values->za = (packed_bytes[4] << 8) | packed_bytes[5];

		// temp in bytes 6 & 7 (skip)

		imu_values->xg = (packed_bytes[8]  << 8) | packed_bytes[9];
		imu_values->yg = (packed_bytes[10] << 8) | packed_bytes[11];
		imu_values->zg = (packed_bytes[12] << 8) | packed_bytes[13];
	}
}

static bool reset_i2c_transaction(hal_i2c_txn_t *txn)
{
	if (!txn)
	{
		return false;
	}

	// Reset control fields
	txn->target_addr = MPU_6050_ADDR;
	txn->i2c_op = HAL_I2C_OP_WRITE;
	txn->expected_bytes_to_tx = 0;
	txn->expected_bytes_to_rx = 0;

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

static bool await_transaction(const hal_i2c_txn_t *txn, uint32_t timeout)
{
	if (!txn)
	{
		return false;
	}

	uint32_t time_start = hal_get_tick();
	while (txn->processing_state != HAL_I2C_TXN_STATE_COMPLETED && !time_util_timed_out(time_start, hal_get_tick(), timeout))
	{
		if (hal_i2c_transaction_servicer() == HAL_STATUS_ERROR)
		{
			return false;
		}
		// short delay
		for (int i = 0; i < 10000; ++i) {}
	}

	return (txn->processing_state == HAL_I2C_TXN_STATE_COMPLETED);
}

static bool whoami()
{
	// configure txn
	if (!reset_i2c_transaction(&i2c_txn))
	{
		// @todo log
		printf("Failed to reset I2C transaction in mpu driver.\n");
		return false;
	}

	// Read the WHOAMI register
	i2c_txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	i2c_txn.expected_bytes_to_tx = 1;
	i2c_txn.expected_bytes_to_rx = 1;
	i2c_txn.tx_data[0] = MPU_WHOAMI_REG;

	// submit txn
	if (hal_i2c_submit_transaction(&i2c_txn) != HAL_STATUS_OK)
	{
		// @todo log
		printf("Failed to submit I2C transaction in mpu driver.\n");
		return false;
	}

	// poll completion
	uint32_t timeout = 10;
	if (!await_transaction(&i2c_txn, timeout))
	{
		// @todo log
		printf("I2C transaction response timed out in mpu driver.\n");
		return false;
	}

	// valididate results
	if (i2c_txn.actual_bytes_transmitted == 1 && i2c_txn.actual_bytes_received == 1 &&
			i2c_txn.rx_data[0] == MPU_6050_ADDR)
	{
		printf("Successfully read whoami register in mpu driver.\r\n");
	}
	else
	{
		printf("Failed to read whoami register in mpu driver.\n");
		return false;
	}

	return true;
}

static bool calculate_factory_trim()
{
	// set up txn
	if (!reset_i2c_transaction(&i2c_txn))
	{
		// @todo log
		printf("Failed to reset I2C transaction in mpu driver.\n");
		return false;
	}

	// burst read the self test registers
	i2c_txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	i2c_txn.expected_bytes_to_tx = 1;
	i2c_txn.expected_bytes_to_rx = 4;
	i2c_txn.tx_data[0] = MPU_SELF_TEST_X_REG;

	// submit txn
	if (hal_i2c_submit_transaction(&i2c_txn) != HAL_STATUS_OK)
	{
		// @todo log
		printf("Failed to submit I2C transaction in mpu driver.\n");
		return false;
	}

	// poll completion
	uint32_t timeout = 10;
	if (!await_transaction(&i2c_txn, timeout))
	{
		// @todo log
		printf("I2C transaction response timed out in mpu driver.\n");
		return false;
	}

	// valididate results
	if (i2c_txn.actual_bytes_transmitted == 1 && i2c_txn.actual_bytes_received == 4)
	{
		self_test_values_t self_test_values = { 0 };
		unpack_self_test(i2c_txn.rx_data, sizeof(i2c_txn.rx_data), &self_test_values);

		// gyro trim
		factory_trim_xg = calc_factory_trim_gyro_x(self_test_values.xg_test);
		factory_trim_yg = calc_factory_trim_gyro_y(self_test_values.yg_test);
		factory_trim_zg = calc_factory_trim_gyro_z(self_test_values.zg_test);

		// accel trim
		factory_trim_xa = calc_factory_trim_accel_x(self_test_values.xa_test);
		factory_trim_ya = calc_factory_trim_accel_y(self_test_values.ya_test);
		factory_trim_za = calc_factory_trim_accel_z(self_test_values.za_test);

		printf("Successfully calculated factory trim in mpu driver.\r\n");
		printf("xg: %0.4f, yg: %0.4f, zg: %0.4f\r\n", factory_trim_xg, factory_trim_yg, factory_trim_zg);
		printf("xa: %0.4f, ya: %0.4f, za: %0.4f\r\n", factory_trim_xa, factory_trim_ya, factory_trim_za);
	}
	else
	{
		printf("Failed to read self test register in mpu driver.\n");
		return false;
	}

	return true;
}

static bool perform_self_test(uint8_t config_reg_addr, uint8_t config_reg)
{
	// set up txn
	if (!reset_i2c_transaction(&i2c_txn))
	{
		// @todo log
		printf("Failed to reset I2C transaction in mpu driver.\n");
		return false;
	}

	// set the value of the config reg
	i2c_txn.i2c_op = HAL_I2C_OP_WRITE;
	i2c_txn.expected_bytes_to_tx = 2;
	i2c_txn.tx_data[0] = config_reg_addr;
	i2c_txn.tx_data[1] = config_reg;

	// submit txn
	if (hal_i2c_submit_transaction(&i2c_txn) != HAL_STATUS_OK)
	{
		// @todo log
		printf("Failed to submit I2C transaction in mpu driver.\n");
		return false;
	}

	// poll completion
	uint32_t timeout = 10;
	if (!await_transaction(&i2c_txn, timeout))
	{
		// @todo log
		printf("I2C transaction response timed out in mpu driver.\n");
		return false;
	}

	// valididate results
	if (i2c_txn.actual_bytes_transmitted != 2)
	{
		return false;
	}

	// set up txn
	if (!reset_i2c_transaction(&i2c_txn))
	{
		// @todo log
		printf("Failed to reset I2C transaction in mpu driver.\n");
		return false;
	}

	// set the value of the config reg
	i2c_txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	i2c_txn.expected_bytes_to_tx = 1;
	i2c_txn.expected_bytes_to_rx = 1;
	i2c_txn.tx_data[0] = config_reg_addr;

	// submit txn
	if (hal_i2c_submit_transaction(&i2c_txn) != HAL_STATUS_OK)
	{
		// @todo log
		printf("Failed to submit I2C transaction in mpu driver.\n");
		return false;
	}

	// poll completion
	if (!await_transaction(&i2c_txn, timeout))
	{
		// @todo log
		printf("I2C transaction response timed out in mpu driver.\n");
		return false;
	}

	// valididate results
	if (!(i2c_txn.actual_bytes_transmitted == 1 && i2c_txn.actual_bytes_received == 1 && i2c_txn.rx_data[0] == config_reg))
	{
		return false;
	}

	// set up txn
	if (!reset_i2c_transaction(&i2c_txn))
	{
		// @todo log
		printf("Failed to reset I2C transaction in mpu driver.\n");
		return false;
	}

	// set the value of the config reg
	i2c_txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	i2c_txn.expected_bytes_to_tx = 1;
	i2c_txn.expected_bytes_to_rx = 14;
	i2c_txn.tx_data[0] = MPU_ACCEL_XOUT_H_REG;

	// submit txn
	if (hal_i2c_submit_transaction(&i2c_txn) != HAL_STATUS_OK)
	{
		// @todo log
		printf("Failed to submit I2C transaction in mpu driver.\n");
		return false;
	}

	// poll completion
	if (!await_transaction(&i2c_txn, timeout))
	{
		// @todo log
		printf("I2C transaction response timed out in mpu driver.\n");
		return false;
	}

	// valididate results
	if (!(i2c_txn.actual_bytes_transmitted == 1 && i2c_txn.actual_bytes_received == 14))
	{
		return false;
	}

	imu_values_t imu_values_under_self_test = { 0 };
	unpack_imu_values(i2c_txn.rx_data, sizeof(i2c_txn.rx_data), &imu_values_under_self_test);

	// unset the self test bit
	uint8_t config_reg_cpy = config_reg;
	config_reg = config_reg & ~0xE0;

	// set up txn
	if (!reset_i2c_transaction(&i2c_txn))
	{
		// @todo log
		printf("Failed to reset I2C transaction in mpu driver.\n");
		return false;
	}

	// set the value of the config reg
	i2c_txn.i2c_op = HAL_I2C_OP_WRITE;
	i2c_txn.expected_bytes_to_tx = 2;
	i2c_txn.tx_data[0] = config_reg_addr;
	i2c_txn.tx_data[1] = config_reg;

	// submit txn
	if (hal_i2c_submit_transaction(&i2c_txn) != HAL_STATUS_OK)
	{
		// @todo log
		printf("Failed to submit I2C transaction in mpu driver.\n");
		return false;
	}

	// poll completion
	if (!await_transaction(&i2c_txn, timeout))
	{
		// @todo log
		printf("I2C transaction response timed out in mpu driver.\n");
		return false;
	}

	// valididate results
	if (i2c_txn.actual_bytes_transmitted != 2)
	{
		return false;
	}

	// set up txn
	if (!reset_i2c_transaction(&i2c_txn))
	{
		// @todo log
		printf("Failed to reset I2C transaction in mpu driver.\n");
		return false;
	}

	// set the value of the config reg
	i2c_txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	i2c_txn.expected_bytes_to_tx = 1;
	i2c_txn.expected_bytes_to_rx = 1;
	i2c_txn.tx_data[0] = config_reg_addr;

	// submit txn
	if (hal_i2c_submit_transaction(&i2c_txn) != HAL_STATUS_OK)
	{
		// @todo log
		printf("Failed to submit I2C transaction in mpu driver.\n");
		return false;
	}

	// poll completion
	if (!await_transaction(&i2c_txn, timeout))
	{
		// @todo log
		printf("I2C transaction response timed out in mpu driver.\n");
		return false;
	}

	// valididate results
	if (!(i2c_txn.actual_bytes_transmitted == 1 && i2c_txn.actual_bytes_received == 1 && i2c_txn.rx_data[0] == config_reg))
	{
		return false;
	}

	// set up txn
	if (!reset_i2c_transaction(&i2c_txn))
	{
		// @todo log
		printf("Failed to reset I2C transaction in mpu driver.\n");
		return false;
	}

	// set the value of the config reg
	i2c_txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	i2c_txn.expected_bytes_to_tx = 1;
	i2c_txn.expected_bytes_to_rx = 14;
	i2c_txn.tx_data[0] = MPU_ACCEL_XOUT_H_REG;

	// submit txn
	if (hal_i2c_submit_transaction(&i2c_txn) != HAL_STATUS_OK)
	{
		// @todo log
		printf("Failed to submit I2C transaction in mpu driver.\n");
		return false;
	}

	// poll completion
	if (!await_transaction(&i2c_txn, timeout))
	{
		// @todo log
		printf("I2C transaction response timed out in mpu driver.\n");
		return false;
	}

	// valididate results
	if (!(i2c_txn.actual_bytes_transmitted == 1 && i2c_txn.actual_bytes_received == 14))
	{
		return false;
	}

	imu_values_t imu_values = { 0 };
	unpack_imu_values(i2c_txn.rx_data, sizeof(i2c_txn.rx_data), &imu_values);

	if (config_reg_addr == MPU_GYRO_CONFIG_REG)
	{
		if (config_reg_cpy & 0x80) // x-axis self test
		{
			float self_test_response = calc_self_test_response(imu_values_under_self_test.xg, imu_values.xg);
			float change_from_factory_percent = calc_change_percent_from_factory_trim(self_test_response, factory_trim_xg);
			printf("xg percent diff: %0.4f\r\n", change_from_factory_percent);
		}
		else if (config_reg_cpy & 0x40) // y-axis self test
		{
			float self_test_response = calc_self_test_response(imu_values_under_self_test.yg, imu_values.yg);
			float change_from_factory_percent = calc_change_percent_from_factory_trim(self_test_response, factory_trim_yg);
			printf("yg percent diff: %0.4f\r\n", change_from_factory_percent);

		}
		else if (config_reg_cpy & 0x20) // z-axis self test
		{
			float self_test_response = calc_self_test_response(imu_values_under_self_test.zg, imu_values.zg);
			float change_from_factory_percent = calc_change_percent_from_factory_trim(self_test_response, factory_trim_zg);
			printf("zg percent diff: %0.4f\r\n", change_from_factory_percent);
		}
	}
	else if (config_reg_addr == MPU_ACCEL_CONFIG_REG)
	{
		if (config_reg_cpy & 0x80) // x-axis self test
		{
			float self_test_response = calc_self_test_response(imu_values_under_self_test.xa, imu_values.xa);
			float change_from_factory_percent = calc_change_percent_from_factory_trim(self_test_response, factory_trim_xa);
			printf("xa percent diff: %0.4f\r\n", change_from_factory_percent);

		}
		else if (config_reg_cpy & 0x40) // y-axis self test
		{
			float self_test_response = calc_self_test_response(imu_values_under_self_test.ya, imu_values.ya);
			float change_from_factory_percent = calc_change_percent_from_factory_trim(self_test_response, factory_trim_ya);
			printf("ya percent diff: %0.4f\r\n", change_from_factory_percent);

		}
		else if (config_reg_cpy & 0x20) // z-axis self test
		{
			float self_test_response = calc_self_test_response(imu_values_under_self_test.za, imu_values.za);
			float change_from_factory_percent = calc_change_percent_from_factory_trim(self_test_response, factory_trim_za);
			printf("za percent diff: %0.4f\r\n", change_from_factory_percent);
		}
	}

	return true;
}

bool imu_mpu6050_init()
{
	if (!whoami())
	{
		return false;
	}

	if (!calculate_factory_trim())
	{
		return false;
	}

	// set up txn
	if (!reset_i2c_transaction(&i2c_txn))
	{
		// @todo log
		printf("Failed to reset I2C transaction in mpu driver.\n");
		return false;
	}

	// set the value of the config reg
	i2c_txn.i2c_op = HAL_I2C_OP_WRITE;
	i2c_txn.expected_bytes_to_tx = 2;
	i2c_txn.tx_data[0] = MPU_PWR_MGMT_1_REG;
	i2c_txn.tx_data[1] = MPU_GYRO_WAKE;

	// submit txn
	if (hal_i2c_submit_transaction(&i2c_txn) != HAL_STATUS_OK)
	{
		// @todo log
		printf("Failed to submit I2C transaction in mpu driver.\n");
		return false;
	}

	// poll completion
	uint32_t timeout = 20;
	if (!await_transaction(&i2c_txn, timeout))
	{
		// @todo log
		printf("I2C HELLO transaction response timed out in mpu driver.\n");
		return false;
	}

	// valididate results
	if (i2c_txn.actual_bytes_transmitted != 2)
	{
		return false;
	}
	// **************************************************************************************
	// self test

	if (!perform_self_test(MPU_ACCEL_CONFIG_REG, 0x90))
	{
		printf("ooh\r\n");
		return false;
	}

	if (!perform_self_test(MPU_ACCEL_CONFIG_REG, 0x50))
	{
		printf("oof\r\n");
		return false;
	}

	if (!perform_self_test(MPU_ACCEL_CONFIG_REG, 0x30))
	{
		return false;
	}

	if (!perform_self_test(MPU_GYRO_CONFIG_REG, 0x80))
	{
		return false;
	}

	if (!perform_self_test(MPU_GYRO_CONFIG_REG, 0x40))
	{
		return false;
	}

	if (!perform_self_test(MPU_GYRO_CONFIG_REG, 0x20))
	{
		return false;
	}
		// for gyro self test, full-scale range should be +/- 250 dps

		// for accel self test, full-scale range should be +/- 8g

		// calc factory trim

		// self test response must be within +/- 14% of factory trim



	// **************************************************************************************
	// config

	// **************************************************************************************
	// wake gyro

	// **************************************************************************************
	// set gyro X as clock source

	return true;
}

/**
 * @brief gets the angular acceleration from the imu
 *
 * @impl LLR_IMU_002
 */
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
