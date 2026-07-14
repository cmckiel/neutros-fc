#include "mpu6050.h"

void mpu6050_unpack_self_test(uint8_t *packed_bytes, uint32_t packed_bytes_size, mpu6050_self_test_values_t *self_test_values)
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

void mpu6050_unpack_imu_values(uint8_t *packed_bytes, uint32_t packed_bytes_size, mpu6050_imu_values_t *imu_values)
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
