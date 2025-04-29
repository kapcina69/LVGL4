#include "lcd_display.h"
#include "rtc_driver.h"
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <lvgl.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <time.h>
#include <sprite_heart.h>
#include <shoe.h>



LOG_MODULE_REGISTER(display_app);

typedef enum {
    DISPLAY_HR,
    DISPLAY_STEPS,
    DISPLAY_TIME,
    DISPLAY_COUNT
} display_view_t;

lv_obj_t *hr_label = NULL;
lv_obj_t *steps_label = NULL;
static lv_obj_t *time_label = NULL;
static display_view_t current_view = DISPLAY_HR;
static lv_timer_t *display_timer = NULL;
static lv_timer_t *time_update_timer = NULL;  // Dodat tajmer za ažuriranje vremena
static lv_obj_t *hr_icon = NULL;
static lv_obj_t *steps_icon = NULL;
static lv_obj_t *time_icon = NULL; 
lv_obj_t *bt_label = NULL;
static lv_obj_t *battery_label = NULL;
static lv_timer_t *blink_timer;
static bool hr_icon_visible = true;





void init_lcd_display(void);
void update_lcd_display(uint32_t hr, uint32_t steps);
void update_time_display(void);
void switch_display_view(lv_timer_t *timer);
void update_time_callback(lv_timer_t *timer);  // Nova callback funkcija

void hr_icon_blink_cb(lv_timer_t *timer)
{
    if (hr_icon_visible) {
        lv_obj_add_flag(hr_icon, LV_OBJ_FLAG_HIDDEN);
        hr_icon_visible = false;
    } else {
        lv_obj_clear_flag(hr_icon, LV_OBJ_FLAG_HIDDEN);
        hr_icon_visible = true;
    }
}
void start_hr_icon_blinking(void)
{
    if (!blink_timer) {
        blink_timer = lv_timer_create(hr_icon_blink_cb, 500, NULL);
    } else {
        lv_timer_resume(blink_timer);
    }
}

void stop_hr_icon_blinking(void)
{
    if (blink_timer) {
        lv_timer_pause(blink_timer);
        // Osiguraj da ikonica ostane prikazana
        // lv_obj_clear_flag(hr_icon, LV_OBJ_FLAG_HIDDEN);
        hr_icon_visible = true;
    }
}



void init_lcd_display(void)
{
    const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
        LOG_ERR("Display not ready");
        return;
    }
    blink_timer = lv_timer_create(hr_icon_blink_cb, 500, NULL); // 500 ms = 0.5s

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_14);

    hr_label = lv_label_create(lv_scr_act());
    lv_obj_add_style(hr_label, &style, 0);
    lv_obj_align(hr_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(hr_label, "HR: --");

    steps_label = lv_label_create(lv_scr_act());
    lv_obj_add_style(steps_label, &style, 0);
    lv_obj_align(steps_label, LV_ALIGN_CENTER, 10, 0);
    lv_label_set_text(steps_label, "Steps: --");
    lv_obj_add_flag(steps_label, LV_OBJ_FLAG_HIDDEN);

    time_label = lv_label_create(lv_scr_act());
    lv_obj_add_style(time_label, &style, 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0,0);
    lv_label_set_text(time_label, "Time: --");
    lv_obj_add_flag(time_label, LV_OBJ_FLAG_HIDDEN);

        // HR ikonica
    hr_icon = lv_img_create(lv_scr_act());
    lv_img_set_src(hr_icon, &Sprite_heart);
    lv_obj_align(hr_icon, LV_ALIGN_CENTER, -30, 0); // Podesi poziciju ručno
    lv_obj_align(hr_label, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_add_flag(hr_icon, LV_OBJ_FLAG_HIDDEN); // Na početku sakrij
    
       // Steps ikonica
    steps_icon = lv_img_create(lv_scr_act());
    lv_img_set_src(steps_icon, &icons8_shoe_print_30);
    lv_obj_align(steps_icon, LV_ALIGN_CENTER, -40, 0); 
    lv_obj_add_flag(steps_icon, LV_OBJ_FLAG_HIDDEN); // Na početku sakrij

        // Bluetooth label
    bt_label = lv_label_create(lv_scr_act());
    lv_label_set_text(bt_label, LV_SYMBOL_BLUETOOTH);
    lv_obj_align(bt_label, LV_ALIGN_TOP_LEFT, 2, 2);
    lv_obj_add_style(bt_label, &style, 0);
    lv_obj_add_flag(bt_label, LV_OBJ_FLAG_HIDDEN);  // Na početku sakrij


    // Battery label
    battery_label = lv_label_create(lv_scr_act());
    lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_FULL);
    lv_obj_align(battery_label, LV_ALIGN_TOP_RIGHT, -2, 2);
    lv_obj_add_style(battery_label, &style, 0);



    display_timer = lv_timer_create(switch_display_view, 3000, NULL);
    time_update_timer = lv_timer_create(update_time_callback, 1000, NULL);  // Ažuriraj vreme svake sekunde

    lv_task_handler();
    display_blanking_off(display_dev);
}

void update_lcd_display(uint32_t hr, uint32_t steps)
{
    char hr_text[64];
    char steps_text[64];


    if (hr != 0) {
        snprintf(hr_text, sizeof(hr_text), "HR: %dbpm", hr);
        lv_label_set_text(hr_label, hr_text);
        lv_obj_align(hr_icon, LV_ALIGN_CENTER, -50, 0); // Podesi poziciju ručno

    }
    
    if (steps != 0) {
        
        snprintf(steps_text, sizeof(steps_text), "Steps: %d", steps);
        lv_label_set_text(steps_label, steps_text);
        if(steps>100 && steps<1000){
            lv_obj_align(steps_icon, LV_ALIGN_CENTER, -45, 0); // Podesi poziciju ručno
        }
        else if(steps>=1000 && steps<10000){
            lv_obj_align(steps_icon, LV_ALIGN_CENTER, -50, 0); // Podesi poziciju ručno
        }
        else if(steps>10000){
            lv_obj_align(steps_icon, LV_ALIGN_CENTER, -55, 0); // Podesi poziciju ručno
        }
            
    }
    
    lv_task_handler();
}


// Nova funkcija koja se poziva svake sekunde da ažurira vreme
void update_time_callback(lv_timer_t *timer)
{
    rtc_update_time_label(time_label);
    if (current_view == DISPLAY_TIME) {
        lv_task_handler();  // Ažuriraj prikaz samo ako je vreme trenutno prikazano
    }
}
void switch_display_view(lv_timer_t *timer)
{
    // Sakrij trenutni prikaz
    switch (current_view) {
        case DISPLAY_HR:
            lv_obj_add_flag(hr_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(hr_icon, LV_OBJ_FLAG_HIDDEN);
            stop_hr_icon_blinking();  // ← dodato
            break;

        case DISPLAY_STEPS:
            lv_obj_add_flag(steps_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(steps_icon, LV_OBJ_FLAG_HIDDEN);
            break;

        case DISPLAY_TIME:
            lv_obj_add_flag(time_label, LV_OBJ_FLAG_HIDDEN);
            break;

        default:
            break;
    }

    // Pređi na sledeći prikaz
    current_view = (current_view + 1) % DISPLAY_COUNT;

    // Prikaži novi prikaz
    switch (current_view) {
        case DISPLAY_HR:
            lv_obj_clear_flag(hr_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(hr_icon, LV_OBJ_FLAG_HIDDEN);
            start_hr_icon_blinking();  // ← pokreće blikovanje samo kad je prikaz aktivan
            break;

        case DISPLAY_STEPS:
            lv_obj_clear_flag(steps_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(steps_icon, LV_OBJ_FLAG_HIDDEN);
            break;

        case DISPLAY_TIME:
            lv_obj_clear_flag(time_label, LV_OBJ_FLAG_HIDDEN);
            break;

        default:
            break;
    }

    lv_task_handler();
}
