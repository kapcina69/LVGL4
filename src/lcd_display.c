#include "lcd_display.h"
#include <lvgl.h>
#include <stdlib.h>  // za rand()
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/sys/printk.h>

lv_obj_t *hr_label = NULL;
lv_obj_t *steps_label = NULL;

void init_lcd_display(void)
{
    const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
        printk("Display not ready\n");
        return;
    }

    // Stil za font
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_14);

    // HR labela (prvi red)
    hr_label = lv_label_create(lv_scr_act());
    lv_obj_add_style(hr_label, &style, 0);
    lv_obj_align(hr_label, LV_ALIGN_TOP_MID, 0, 10);

    // Steps labela (drugi red)
    steps_label = lv_label_create(lv_scr_act());
    lv_obj_add_style(steps_label, &style, 0);
    lv_obj_align(steps_label, LV_ALIGN_TOP_MID, 0, 30);

    lv_task_handler();
    display_blanking_off(display_dev);
}

void update_lcd_display(uint32_t hr, uint32_t steps)
{
    char hr_text[32];
    char steps_text[32];

    snprintf(hr_text, sizeof(hr_text), "HR: %d", hr);
    snprintf(steps_text, sizeof(steps_text), "Steps: %d", steps);

    lv_label_set_text(hr_label, hr_text);
    lv_label_set_text(steps_label, steps_text);

    lv_task_handler();
}
