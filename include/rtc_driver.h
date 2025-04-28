#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <lvgl.h>

/* Initialize RTC hardware */
int rtc_init(void);

/* Set initial time in RTC */
void rtc_set_initial_time(const char *data);

/* Get current time and update LVGL label */
void rtc_update_time_label(lv_obj_t *time_label);

#endif /* RTC_DRIVER_H */