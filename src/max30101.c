#include "max30101.h"
#include <zephyr/sys/printk.h>
#include <math.h>

static bool sensor_initialized = false;
static const struct device *i2c_dev;

// I2C functions
static int max30101_reg_write(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    return i2c_write(i2c_dev, buf, sizeof(buf), MAX30101_I2C_ADDR);
}

static int max30101_reg_read(uint8_t reg, uint8_t *val) {
    return i2c_write_read(i2c_dev, MAX30101_I2C_ADDR, &reg, 1, val, 1);
}

static int max30101_block_read(uint8_t reg, uint8_t *buf, uint8_t len) {
    return i2c_write_read(i2c_dev, MAX30101_I2C_ADDR, &reg, 1, buf, len);
}

static void max30101_reset() {
    max30101_reg_write(REG_MODE_CONFIG, 0x40);
    k_sleep(K_MSEC(100));
}

static int max30101_check_id() {
    uint8_t part_id;
    int ret = max30101_reg_read(REG_PART_ID, &part_id);

    printk("Read part ID result = %d, value = 0x%02X\n", ret, part_id);

    if (max30101_reg_read(REG_PART_ID, &part_id) != 0 || part_id != 0x15) {
        printk("Sensor not found!\n");
        return -1;
    }
    return 0;
}

int max30101_init(const struct device *i2c_device) {
    i2c_dev = i2c_device;
    
    if (max30101_check_id() != 0) return -1;

    max30101_reset();
    max30101_reg_write(REG_FIFO_CONFIG, FIFO_ALMOST_FULL);
    max30101_reg_write(REG_MODE_CONFIG, SPO2_MODE);
    
    uint8_t spo2_config = (SAMPLE_RATE_100HZ << 2) | LED_PULSE_WIDTH_16BIT;
    max30101_reg_write(REG_SPO2_CONFIG, spo2_config);
    
    max30101_reg_write(REG_LED2_PA, LED_CURRENT_37MA); // Only need IR LED
    max30101_reg_write(REG_MULTI_LED_CTRL1, 0x02);    // Only LED2 (IR) in FIFO
    
    sensor_initialized = true;
    printk("MAX30101 initialized\n");
    return 0;
}

int max30101_read_ir_sample(uint32_t *ir) {
    if (!sensor_initialized) return -1;

    uint8_t buf[6];
    if (max30101_block_read(REG_FIFO_DATA, buf, sizeof(buf))) {
        return -1;
    }

    *ir = (buf[3] << 16) | (buf[4] << 8) | buf[5];
    *ir &= 0x3FFFF;
    return 0;
}

int count_peaks(uint32_t *samples, int count) {
    const int window_size = 40;
    const uint32_t threshold = 100000;

    int peaks = 0;
    int prev_peak_index = -1;
    int last_peak_index = -window_size * 2;

    for (int i = window_size; i < count - window_size; i++) {
        if (samples[i] < threshold) continue;

        bool is_peak = true;

        // Check if it's the maximum in the window
        for (int j = 1; j <= window_size; j++) {
            if (samples[i] <= samples[i - j] || samples[i] <= samples[i + j]) {
                is_peak = false;
                break;
            }
        }

        if (!is_peak) continue;

        if ((i - last_peak_index) > window_size) {
            prev_peak_index = last_peak_index;
            last_peak_index = i;
            peaks++;
        }
    }

    if (peaks < 2 || prev_peak_index < 0) {
        return 0;
    }

    return (last_peak_index - prev_peak_index);
}