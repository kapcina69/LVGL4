#include "rtc_driver.h"
#include <zephyr/sys/printk.h>
#include <time.h>
#include <stdio.h>

static const struct device *rtc_dev = DEVICE_DT_GET(DT_NODELABEL(rtc0));

int rtc_init(void)
{
    if (!device_is_ready(rtc_dev)) {
        printk("RTC device not ready\n");
        return -ENODEV;
    }

    int ret = counter_start(rtc_dev);
    if (ret != 0) {
        printk("Failed to start counter: %d\n", ret);
        return ret;
    }

    printk("RTC initialized successfully\n");
    return 0;
}



void rtc_set_initial_time(const char *data)
{
    if (data == NULL) {
        printk("Error: data is NULL\n");
        return;
    }

    if (strlen(data) < 19) {  // Proveri da li string ima minimalnu duzinu "YYYY-MM-DD HH:MM:SS"
        printk("Error: Invalid data length\n");
        return;
    }

    struct tm tm_time = {0};
    int scanned = sscanf(data, "%d-%d-%d %d:%d:%d",
                          &tm_time.tm_year,
                          &tm_time.tm_mon,
                          &tm_time.tm_mday,
                          &tm_time.tm_hour,
                          &tm_time.tm_min,
                          &tm_time.tm_sec);

    if (scanned != 6) {
        printk("Error: Failed to parse data\n");
        return;
    }

    tm_time.tm_year -= 1900;  // Godina je broj godina od 1900.
    tm_time.tm_mon -= 1;      // Mesec je 0-based (januar je 0, februar je 1,...)

    time_t new_time = mktime(&tm_time);
    struct timespec ts = {.tv_sec = new_time};

    if (clock_settime(CLOCK_REALTIME, &ts) < 0) {
        printk("Failed to set initial time\n");
    } else {
        printk("Initial time set to: %04d-%02d-%02d %02d:%02d:%02d\n",
               tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
               tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
}



void rtc_update_time_label(lv_obj_t *time_label)
{
    struct timespec ts;
    char time_text[48];

    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
        printk("Failed to get time\n");
        lv_label_set_text(time_label, "Time error");
        return;
    }

    struct tm *timeinfo = gmtime(&ts.tv_sec);
    if (timeinfo == NULL) {
        printk("Failed to convert time\n");
        lv_label_set_text(time_label, "Time error");
        return;
    }

    static const char *weekday_names[] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };

    static const char *month_names[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    snprintf(time_text, sizeof(time_text),
             "  %s, %02d %s %04d\n     %02d:%02d:%02d",
             weekday_names[timeinfo->tm_wday],
             timeinfo->tm_mday,
             month_names[timeinfo->tm_mon],
             timeinfo->tm_year + 1900,
             timeinfo->tm_hour,
             timeinfo->tm_min,
             timeinfo->tm_sec);

    lv_label_set_text(time_label, time_text);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 10);

    printk("Current time: %s\n", time_text);
}

