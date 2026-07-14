#include "mpu6050_init.h"
#include "mpu6050.h"
#include "hal/i2c.h"
#include "hal/systick.h"
#include "time_util.h"

#include <string.h>
#include <math.h>

/***************************************************/
// PRIVATE TYPES
/***************************************************/

/***************************************************/
// PRIVATE DATA
/***************************************************/
static hal_i2c_txn_t txn = { 0 };

// Factory Trim Gyroscope
static float factory_trim_xg = 0;
static float factory_trim_yg = 0;
static float factory_trim_zg = 0;

// Factory Trim Accelerometer
static float factory_trim_xa = 0;
static float factory_trim_ya = 0;
static float factory_trim_za = 0;

// Self Test Response Gyro
static float xg_self_test_response = 0;
static float yg_self_test_response = 0;
static float zg_self_test_response = 0;

// Self Test Response Accel
static float xa_self_test_response = 0;
static float ya_self_test_response = 0;
static float za_self_test_response = 0;

// Percent Difference From Factory Gyro
static float xg_percent_diff_from_factory = 0;
static float yg_percent_diff_from_factory = 0;
static float zg_percent_diff_from_factory = 0;

// Percent Difference From Factory Accel
static float xa_percent_diff_from_factory = 0;
static float ya_percent_diff_from_factory = 0;
static float za_percent_diff_from_factory = 0;

/***************************************************/
// PRIVATE FUNCTIONS
/***************************************************/
static void reset_i2c_transaction()
{
	// Reset control fields
	txn.target_addr = MPU_6050_ADDR;
	txn.i2c_op = HAL_I2C_OP_READ;
	txn.expected_bytes_to_tx = 0;
	txn.expected_bytes_to_rx = 0;

	// Reset buffers
	memset(txn.tx_data, 0, sizeof(txn.tx_data));
	memset(txn.rx_data, 0, sizeof(txn.rx_data));

	// Reset result fields
	txn.processing_state = HAL_I2C_TXN_STATE_CREATED;
	txn.transaction_result = HAL_I2C_TXN_RESULT_NONE;
	txn.actual_bytes_received = 0;
	txn.actual_bytes_transmitted = 0;
}

/******************** CALC GYRO TRIM ***********************/
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

/******************** CALC ACCEL TRIM ***********************/
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

/******************** SELF TEST RESPONSE ***********************/
static float calc_self_test_response(float gyro_out_with_self_test_enabled, float gyro_out_with_self_test_disabled)
{
	return gyro_out_with_self_test_enabled - gyro_out_with_self_test_disabled;
}

/******************** PERCENT DIFF FROM FACTORY ***********************/
static float calc_change_percent_from_factory_trim(float self_test_response, float factory_trim)
{
	if (factory_trim == 0)
	{
		return 0;
	}

	return (self_test_response - factory_trim) / factory_trim;
}

/******************** Performing I2C Transactions ***********************/
static bool await_transaction(uint32_t timeout)
{
	uint32_t time_start = hal_get_tick();
	while (txn.processing_state != HAL_I2C_TXN_STATE_COMPLETED && !time_util_timed_out(time_start, hal_get_tick(), timeout))
	{
		if (hal_i2c_transaction_servicer() == HAL_STATUS_ERROR)
		{
			return false;
		}
		// short delay
		for (int i = 0; i < 10000; ++i) {}
	}

	return (txn.processing_state == HAL_I2C_TXN_STATE_COMPLETED);
}

static bool perform_transaction(uint32_t timeout)
{
	return hal_i2c_submit_transaction(&txn) == HAL_STATUS_OK && await_transaction(timeout);
}

/******************** Factory Trim ***********************/
static bool calculate_factory_trim()
{
	reset_i2c_transaction();

	// burst read the self test registers
	txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	txn.expected_bytes_to_tx = 1;
	txn.expected_bytes_to_rx = 4;
	txn.tx_data[0] = MPU_SELF_TEST_X_REG;

	uint32_t timeout = 10;
	if (!perform_transaction(timeout))
	{
		// @todo log
		return false;
	}

	// valididate results
	if (txn.actual_bytes_transmitted == 1 && txn.actual_bytes_received == 4)
	{
		mpu6050_self_test_values_t self_test_values = { 0 };
		mpu6050_unpack_self_test(txn.rx_data, sizeof(txn.rx_data), &self_test_values);

		// gyro trim
		factory_trim_xg = calc_factory_trim_gyro_x(self_test_values.xg_test);
		factory_trim_yg = calc_factory_trim_gyro_y(self_test_values.yg_test);
		factory_trim_zg = calc_factory_trim_gyro_z(self_test_values.zg_test);

		// accel trim
		factory_trim_xa = calc_factory_trim_accel_x(self_test_values.xa_test);
		factory_trim_ya = calc_factory_trim_accel_y(self_test_values.ya_test);
		factory_trim_za = calc_factory_trim_accel_z(self_test_values.za_test);
	}
	else
	{
		return false;
	}

	return true;
}

/******************** Self Test for a given axis ***********************/
static bool mpu6050_perform_self_test_on_axis(uint8_t config_reg_addr, uint8_t config_reg_val)
{
	// ******************* Set the config register value **********************
	reset_i2c_transaction();

	// message to set the value of the config reg
	txn.i2c_op = HAL_I2C_OP_WRITE;
	txn.expected_bytes_to_tx = 2;
	txn.tx_data[0] = config_reg_addr;
	txn.tx_data[1] = config_reg_val;

	uint32_t timeout = 10;
	if (!perform_transaction(timeout))
	{
		return false;
	}

	// valididate results
	if (txn.actual_bytes_transmitted != 2)
	{
		return false;
	}

	// ******************* Confirm the config register value **********************
	reset_i2c_transaction();

	// message to confirm the value of the config reg
	txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	txn.expected_bytes_to_tx = 1;
	txn.expected_bytes_to_rx = 1;
	txn.tx_data[0] = config_reg_addr;

	if (!perform_transaction(timeout))
	{
		return false;
	}

	// valididate results
	if (!(txn.actual_bytes_transmitted == 1 && txn.actual_bytes_received == 1 && txn.rx_data[0] == config_reg_val))
	{
		return false;
	}

	// ******************* Burst read the gyro, temp, and accel while in self test **********************
	reset_i2c_transaction();

	// message to burst read gyro, temp, and accel
	txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	txn.expected_bytes_to_tx = 1;
	txn.expected_bytes_to_rx = 14;
	txn.tx_data[0] = MPU_ACCEL_XOUT_H_REG;

	if (!perform_transaction(timeout))
	{
		return false;
	}

	// valididate results
	if (!(txn.actual_bytes_transmitted == 1 && txn.actual_bytes_received == 14))
	{
		return false;
	}

	// process results: unpack the data
	// Will be used later to compare the same readings outside of test mode.
	mpu6050_imu_values_t imu_values_under_self_test = { 0 };
	mpu6050_unpack_imu_values(txn.rx_data, sizeof(txn.rx_data), &imu_values_under_self_test);

	// ******************* Exit self test mode **********************
	// remember which axis test bit was set prior to removing self test bit
	bool x_self_test = (config_reg_val & 0x80) >> 7;
	bool y_self_test = (config_reg_val & 0x40) >> 6;
	bool z_self_test = (config_reg_val & 0x20) >> 5;

	// unset the self test bit in prep to disable self test mode
	config_reg_val = config_reg_val & ~0xE0;

	reset_i2c_transaction();

	// message to disable self test
	txn.i2c_op = HAL_I2C_OP_WRITE;
	txn.expected_bytes_to_tx = 2;
	txn.tx_data[0] = config_reg_addr;
	txn.tx_data[1] = config_reg_val;

	if (!perform_transaction(timeout))
	{
		return false;
	}

	// valididate results
	if (txn.actual_bytes_transmitted != 2)
	{
		return false;
	}

	// ******************* Confirm exit self test mode **********************
	reset_i2c_transaction();

	// message to confirm self test mode disabled
	txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	txn.expected_bytes_to_tx = 1;
	txn.expected_bytes_to_rx = 1;
	txn.tx_data[0] = config_reg_addr;

	if (!perform_transaction(timeout))
	{
		return false;
	}

	// valididate results
	if (!(txn.actual_bytes_transmitted == 1 && txn.actual_bytes_received == 1 && txn.rx_data[0] == config_reg_val))
	{
		return false;
	}

	// ******************* Burst read gyro, temp, and accel outside of self test mode **********************
	reset_i2c_transaction();

	// message to burst read gryo, temp, accel
	txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	txn.expected_bytes_to_tx = 1;
	txn.expected_bytes_to_rx = 14;
	txn.tx_data[0] = MPU_ACCEL_XOUT_H_REG;

	if (!perform_transaction(timeout))
	{
		return false;
	}

	// valididate results
	if (!(txn.actual_bytes_transmitted == 1 && txn.actual_bytes_received == 14))
	{
		return false;
	}

	// process results
	mpu6050_imu_values_t imu_values = { 0 };
	mpu6050_unpack_imu_values(txn.rx_data, sizeof(txn.rx_data), &imu_values);

	// ******************* Perform the self test now that all data is here **********************

	if (config_reg_addr == MPU_GYRO_CONFIG_REG)
	{
		if (x_self_test) // x-axis self test
		{
			xg_self_test_response = calc_self_test_response(imu_values_under_self_test.xg, imu_values.xg);
			xg_percent_diff_from_factory = calc_change_percent_from_factory_trim(xg_self_test_response, factory_trim_xg);
		}
		else if (y_self_test) // y-axis self test
		{
			yg_self_test_response = calc_self_test_response(imu_values_under_self_test.yg, imu_values.yg);
			yg_percent_diff_from_factory = calc_change_percent_from_factory_trim(yg_self_test_response, factory_trim_yg);

		}
		else if (z_self_test) // z-axis self test
		{
			zg_self_test_response = calc_self_test_response(imu_values_under_self_test.zg, imu_values.zg);
			zg_percent_diff_from_factory = calc_change_percent_from_factory_trim(zg_self_test_response, factory_trim_zg);
		}
	}
	else if (config_reg_addr == MPU_ACCEL_CONFIG_REG)
	{
		if (x_self_test) // x-axis self test
		{
			xa_self_test_response = calc_self_test_response(imu_values_under_self_test.xa, imu_values.xa);
			xa_percent_diff_from_factory = calc_change_percent_from_factory_trim(xa_self_test_response, factory_trim_xa);

		}
		else if (y_self_test) // y-axis self test
		{
			ya_self_test_response = calc_self_test_response(imu_values_under_self_test.ya, imu_values.ya);
			ya_percent_diff_from_factory = calc_change_percent_from_factory_trim(ya_self_test_response, factory_trim_ya);

		}
		else if (z_self_test) // z-axis self test
		{
			za_self_test_response = calc_self_test_response(imu_values_under_self_test.za, imu_values.za);
			za_percent_diff_from_factory = calc_change_percent_from_factory_trim(za_self_test_response, factory_trim_za);
		}
	}

	return true;
}

/***************************************************/
// PUBLIC FUNCTIONS
/***************************************************/
bool mpu6050_whoami()
{
	uint32_t timeout = 10;

	reset_i2c_transaction();

	// Read the WHOAMI register
	txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	txn.expected_bytes_to_tx = 1;
	txn.expected_bytes_to_rx = 1;
	txn.tx_data[0] = MPU_WHOAMI_REG;

	if (!perform_transaction(timeout))
	{
		// @todo log
		return false;
	}

	// Validate
	if (txn.actual_bytes_transmitted != 1 || txn.actual_bytes_received != 1 ||
			txn.rx_data[0] != MPU_6050_ADDR)
	{
		return false;
	}

	return true;
}

bool mpu6050_set_clock_source_gyro_x()
{
	reset_i2c_transaction();

	// message to set the internal clock source to gyro x-axis
	txn.i2c_op = HAL_I2C_OP_WRITE;
	txn.expected_bytes_to_tx = 2;
	txn.tx_data[0] = MPU_PWR_MGMT_1_REG;
	txn.tx_data[1] = MPU_GYRO_WAKE;

	uint32_t timeout = 10;
	if (!perform_transaction(timeout))
	{
		// @todo log
		return false;
	}
	// valididate results
	if (txn.actual_bytes_transmitted != 2)
	{
		return false;
	}

	reset_i2c_transaction();

	// message to set the internal clock source to gyro x-axis
	txn.i2c_op = HAL_I2C_OP_WRITE_READ;
	txn.expected_bytes_to_rx = 1;
	txn.expected_bytes_to_tx = 1;
	txn.tx_data[0] = MPU_PWR_MGMT_1_REG;

	if (!perform_transaction(timeout))
	{
		// @todo log
		return false;
	}

	// valididate results
	// @todo the last check isn't accurate and can cause problems. The power management reg has multiple bits and
	// doesn't have to strictly equal MPU_GYRO_WAKE as described here. Fix!!
	if (txn.actual_bytes_transmitted != 1 || txn.actual_bytes_received != 1 || txn.rx_data[0] != MPU_GYRO_WAKE)
	{
		return false;
	}

	return true;
}

bool mpu6050_perform_self_test(float *percent_diff_ft_xg, float *percent_diff_ft_yg, float *percent_diff_ft_zg,
															 float *percent_diff_ft_xa, float *percent_diff_ft_ya, float *percent_diff_ft_za)
{
	if (!calculate_factory_trim())
	{
		// @todo log
		return false;
	}

	// for accel self test, full-scale range should be +/- 8g
	if (!mpu6050_perform_self_test_on_axis(MPU_ACCEL_CONFIG_REG, 0x90))
	{
		return false;
	}

	if (!mpu6050_perform_self_test_on_axis(MPU_ACCEL_CONFIG_REG, 0x50))
	{
		return false;
	}

	if (!mpu6050_perform_self_test_on_axis(MPU_ACCEL_CONFIG_REG, 0x30))
	{
		return false;
	}

	// for gyro self test, full-scale range should be +/- 250 dps
	if (!mpu6050_perform_self_test_on_axis(MPU_GYRO_CONFIG_REG, 0x80))
	{
		return false;
	}

	if (!mpu6050_perform_self_test_on_axis(MPU_GYRO_CONFIG_REG, 0x40))
	{
		return false;
	}

	if (!mpu6050_perform_self_test_on_axis(MPU_GYRO_CONFIG_REG, 0x20))
	{
		return false;
	}

	if (!(percent_diff_ft_xg && percent_diff_ft_yg && percent_diff_ft_zg &&
			percent_diff_ft_xa && percent_diff_ft_ya && percent_diff_ft_za))
	{
		return false;
	}

	*percent_diff_ft_xg = xg_percent_diff_from_factory;
	*percent_diff_ft_yg = yg_percent_diff_from_factory;
	*percent_diff_ft_zg = zg_percent_diff_from_factory;

	*percent_diff_ft_xa = xa_percent_diff_from_factory;
	*percent_diff_ft_ya = ya_percent_diff_from_factory;
	*percent_diff_ft_za = za_percent_diff_from_factory;

	return true;
}
