#include "max30101.h"
#include "bmi160.h"
#include "rtc_driver.h"
#include "lcd_display.h"
#include "ble_service.h"
#include "ble_nus.h"
#include "ble_hrs.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <time.h>

#define BUFFER_SIZE 500
static uint32_t ir_buffer[BUFFER_SIZE];
static int buffer_index = 0;
char *received_data_from_bluetooth;
bool value_nrf_connect=false;
// Thread stacks and control structures
K_THREAD_STACK_DEFINE(max30101_thread_stack, 5000);
K_THREAD_STACK_DEFINE(bmi160_thread_stack, 5000);
K_THREAD_STACK_DEFINE(display_thread_stack, 4096);
K_THREAD_STACK_DEFINE(bluetooth_thread_stack, 4096);
static struct k_thread max30101_thread_data;
static struct k_thread bmi160_thread_data;
static struct k_thread display_thread_data;
static struct k_thread bluetooth_thread_data;

// Message queues
K_MSGQ_DEFINE(hr_msgq, sizeof(int), 10, 4);

struct step_data {
    uint16_t steps;
    struct sensor_value accel[3];
};
K_MSGQ_DEFINE(step_msgq, sizeof(struct step_data), 10, 4);

// Display data structure
struct display_data {
    uint32_t hr;
    uint32_t steps;
};
K_MSGQ_DEFINE(display_msgq, sizeof(struct display_data), 10, 4);

// Bluetooth data structure
struct bluetooth_data {
    uint32_t hr;
    uint32_t steps;
};
K_MSGQ_DEFINE(bluetooth_msgq, sizeof(struct bluetooth_data), 10, 4);

void max30101_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (1) {
        uint32_t ir;
        if (max30101_read_ir_sample(&ir) == 0) {
            ir_buffer[buffer_index++] = ir;

            if (buffer_index >= BUFFER_SIZE) {
                int delta = count_peaks(ir_buffer, BUFFER_SIZE);
                int heart_rate = 0;

                if (delta <= 0) {
                    printk("Finger removed\n");
                } else {
                    heart_rate = (60 * 100) / delta;
                    if(heart_rate > 40) {
                        k_msgq_put(&hr_msgq, &heart_rate, K_NO_WAIT);
                    }
                }
                buffer_index = 0;
            }
        }
        k_sleep(K_MSEC(10));
    }
}

void bmi160_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);
    
    const struct device *sensor = DEVICE_DT_GET_ONE(bosch_bmi160);
    if (!device_is_ready(sensor)) {
        printk("BMI160 device not ready\n");
        return;
    }

    if (bmi160_enable_step_counter() != 0) {
        printk("Failed to enable step counter\n");
        return;
    }

    while (1) {
        uint16_t steps = 0;
        struct sensor_value accel[3];
        struct step_data step_data;

        if (sensor_sample_fetch(sensor) == 0) {
            if (sensor_channel_get(sensor, SENSOR_CHAN_ACCEL_XYZ, accel) == 0) {
                if (bmi160_read_step_count(&steps) == 0) {
                    step_data.steps = steps;
                    printk("Steps1: %u\n", steps);
                    memcpy(step_data.accel, accel, sizeof(accel));
                    k_msgq_put(&step_msgq, &step_data, K_NO_WAIT);
                }
            }
        }
        k_sleep(K_MSEC(1000));
    }
}

void display_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    // Initialize display
    init_lcd_display();
    
    struct display_data data = {0};
    
    while (1) {
        // Get new data from queue with timeout
        if (k_msgq_get(&display_msgq, &data, K_MSEC(100)) == 0) {
            update_lcd_display(data.hr, data.steps);
        }
        
        // Always refresh the display periodically
        lv_task_handler();
        k_sleep(K_MSEC(50));  // Refresh at ~20Hz
    }
}

void bluetooth_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    struct bluetooth_data data;
    static uint32_t last_hr = 0;
    static uint32_t last_steps = 0;

    while (1) {
        if (k_msgq_get(&bluetooth_msgq, &data, K_MSEC(100)) == 0) {
            // Only send data if it has changed
            if (data.hr != last_hr || data.steps != last_steps) {
                ble_nus_send_data(data.hr, data.steps);
                ble_hrs_update(data.hr);
                last_hr = data.hr;
                last_steps = data.steps;
            }
        }
        k_sleep(K_MSEC(100));
    }
}
void handle_received_data(const char *data)
{
    printk("App received: %s\n", data);

    // Process the received data here
    received_data_from_bluetooth=data;
    value_nrf_connect=true;
    
}

void main(void) 
{
    // Initialize Bluetooth
    ble_init(handle_received_data);
    int ret = rtc_init();
    if (ret != 0) {
        return ret;
    }
    printk("RTC initialized:%s\n", received_data_from_bluetooth);
    rtc_set_initial_time(received_data_from_bluetooth);

    // rtc_set_initial_time();
    
    if (ble_hrs_init() != 0) {
        printk("Failed to initialize BLE HRS!\n");
    }

    printk("MAX30101 Heart Rate Monitor with BMI160\n");
    
    const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    if (!device_is_ready(i2c_dev)) {
        printk("I2C for MAX not ready!\n");
        return;
    }

    if (max30101_init(i2c_dev) != 0) {
        printk("Sensor init failed!\n");
        return;
    }

    // Create threads
    k_thread_create(&max30101_thread_data, max30101_thread_stack,
                   K_THREAD_STACK_SIZEOF(max30101_thread_stack),
                   max30101_thread,
                   NULL, NULL, NULL,
                   5, 0, K_NO_WAIT);

    k_thread_create(&bmi160_thread_data, bmi160_thread_stack,
                   K_THREAD_STACK_SIZEOF(bmi160_thread_stack),
                   bmi160_thread,
                   NULL, NULL, NULL,
                   5, 0, K_NO_WAIT);

    k_thread_create(&display_thread_data, display_thread_stack,
                   K_THREAD_STACK_SIZEOF(display_thread_stack),
                   display_thread,
                   NULL, NULL, NULL,
                   4, 0, K_NO_WAIT);  // Higher priority than sensor threads

    k_thread_create(&bluetooth_thread_data, bluetooth_thread_stack,
                   K_THREAD_STACK_SIZEOF(bluetooth_thread_stack),
                   bluetooth_thread,
                   NULL, NULL, NULL,
                   3, 0, K_NO_WAIT);  // Higher priority than sensor threads, lower than display
    char *received_data_from_bluetooth1; 
    while (1) {
        int heart_rate;
        struct step_data step_data;
        struct display_data display_data = {0};
        struct bluetooth_data bt_data = {0};
        // printk("Bluetooth data: %s\n", received_data_from_bluetooth);
        // printk("RTC data: %s\n", received_data_from_bluetooth1);
        if(value_nrf_connect==true) {
          printk("RTC initialized:%s\n", received_data_from_bluetooth);
          rtc_set_initial_time(received_data_from_bluetooth);
            value_nrf_connect=false;

        }

        if (k_msgq_get(&hr_msgq, &heart_rate, K_NO_WAIT) == 0) {
            printk("HR: %d bpm\n", heart_rate);
            display_data.hr = heart_rate;
            bt_data.hr = heart_rate;
        }

        if (k_msgq_get(&step_msgq, &step_data, K_NO_WAIT) == 0) {
            printk("Steps: %u\n", step_data.steps);
            display_data.steps = step_data.steps;
            bt_data.steps = step_data.steps;
        }

        if (display_data.hr != 0 || display_data.steps != 0) {
            k_msgq_put(&display_msgq, &display_data, K_NO_WAIT);
            k_msgq_put(&bluetooth_msgq, &bt_data, K_NO_WAIT);
        }

        k_sleep(K_MSEC(100));
    }
}