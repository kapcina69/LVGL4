#include "lcd_display.h"
#include "rtc_driver.h"
#include <stdlib.h>  // za rand()
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <lvgl.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <time.h>

LOG_MODULE_REGISTER(display_app);

// Definicije za prikaze
typedef enum {
    DISPLAY_HR,
    DISPLAY_STEPS,
    DISPLAY_TIME,
    DISPLAY_COUNT  // Broj prikaza
} display_view_t;

// Globalne promenljive
 lv_obj_t *hr_label = NULL;
 lv_obj_t *steps_label = NULL;
static lv_obj_t *time_label = NULL;
static display_view_t current_view = DISPLAY_HR;
static lv_timer_t *display_timer = NULL;

// Prototipovi funkcija
void init_lcd_display(void);
void update_lcd_display(uint32_t hr, uint32_t steps);
void update_time_display(void);
void switch_display_view(lv_timer_t *timer);

// Inicijalizacija displeja
void init_lcd_display(void)
{
    const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
        LOG_ERR("Display not ready");
        return;
    }

    // Stil za font
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_14);

    // Kreiranje labela - svi su centrirani na ekranu
    hr_label = lv_label_create(lv_scr_act());
    lv_obj_add_style(hr_label, &style, 0);
    lv_obj_align(hr_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(hr_label, "HR: --");

    steps_label = lv_label_create(lv_scr_act());
    lv_obj_add_style(steps_label, &style, 0);
    lv_obj_align(steps_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(steps_label, "Steps: --");
    lv_obj_add_flag(steps_label, LV_OBJ_FLAG_HIDDEN);

    time_label = lv_label_create(lv_scr_act());
    lv_obj_add_style(time_label, &style, 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(time_label, "Time: --");
    lv_obj_add_flag(time_label, LV_OBJ_FLAG_HIDDEN);

    // Kreiranje tajmera za promenu prikaza (3000 ms = 3 sekunde)
    display_timer = lv_timer_create(switch_display_view, 3000, NULL);

    lv_task_handler();
    display_blanking_off(display_dev);
}

// Ažuriranje prikaza srca
void update_lcd_display(uint32_t hr, uint32_t steps)
{
    char hr_text[32];
    char steps_text[32];
    
    if (hr != 0) {
        snprintf(hr_text, sizeof(hr_text), "HR: %d", hr);
        lv_label_set_text(hr_label, hr_text);
    }
    
    if (steps != 0) {
        snprintf(steps_text, sizeof(steps_text), "Steps: %d", steps);
        lv_label_set_text(steps_label, steps_text);
    }
    
    lv_task_handler();
}

#include <zephyr/drivers/rtc.h>
#include <zephyr/sys/timeutil.h>



// Promena prikaza - poziva se na svaki tajmer
void switch_display_view(lv_timer_t *timer)
{
    // Sakrij trenutni prikaz
    switch (current_view) {
        case DISPLAY_HR:
            lv_obj_add_flag(hr_label, LV_OBJ_FLAG_HIDDEN);
            break;
        case DISPLAY_STEPS:
            lv_obj_add_flag(steps_label, LV_OBJ_FLAG_HIDDEN);
            break;
        case DISPLAY_TIME:
            lv_obj_add_flag(time_label, LV_OBJ_FLAG_HIDDEN);
            break;
        default:
            break;
    }

    // Pređi na sledeći prikaz
    current_view = (current_view + 1) % DISPLAY_COUNT;

    // Prikaži novi prikaz i ažuriraj ga ako je potrebno
    switch (current_view) {
        case DISPLAY_HR:
            lv_obj_clear_flag(hr_label, LV_OBJ_FLAG_HIDDEN);
            break;
        case DISPLAY_STEPS:
            lv_obj_clear_flag(steps_label, LV_OBJ_FLAG_HIDDEN);
            break;
        case DISPLAY_TIME:
            rtc_update_time_label(time_label);
            lv_obj_clear_flag(time_label, LV_OBJ_FLAG_HIDDEN);
            break;
        default:
            break;
    }
}