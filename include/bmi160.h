#ifndef BMI160_H_
#define BMI160_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>

#define BMI160_I2C_ADDR 0x69
// extern const struct device *i2c_dev;


int bmi160_enable_step_counter(void);
int bmi160_read_step_count(uint16_t *steps);
int bmi160_reset_step_counter(void);

#endif // BMI160_H_
