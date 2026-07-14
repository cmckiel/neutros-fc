#ifndef _MPU6050_H
#define _MPU6050_H

#include "stdint.h"

typedef struct {
	uint8_t xg_test;
	uint8_t yg_test;
	uint8_t zg_test;

	uint8_t xa_test;
	uint8_t ya_test;
	uint8_t za_test;
} mpu6050_self_test_values_t;

typedef struct {
	uint16_t xg;
	uint16_t yg;
	uint16_t zg;

	uint16_t xa;
	uint16_t ya;
	uint16_t za;
} mpu6050_imu_values_t;

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

void mpu6050_unpack_self_test(uint8_t *packed_bytes, uint32_t packed_bytes_size, mpu6050_self_test_values_t *self_test_values);
void mpu6050_unpack_imu_values(uint8_t *packed_bytes, uint32_t packed_bytes_size, mpu6050_imu_values_t *imu_values);

#endif /* _MPU6050_H */
