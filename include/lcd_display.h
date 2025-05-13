#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <lvgl.h>

// Display views
typedef enum {
    DISPLAY_HR,
    DISPLAY_STEPS,
    DISPLAY_TIME,
    DISPLAY_COUNT,
    DISPLAY_ALL
} display_view_t;

// External variables
extern lv_obj_t *hr_label;
extern lv_obj_t *steps_label;
extern lv_obj_t *time_label;
extern lv_obj_t *bt_label;
extern lv_obj_t *hr_icon;
extern lv_obj_t *steps_icon;

extern int display_timer_period;
extern lv_timer_t *display_timer;
extern display_view_t current_view;

// Function declarations
void init_lcd_display(void);
void update_lcd_display(uint32_t hr, uint32_t steps);
void reset(void);
void change_display_view(char *message);
void charging_view(void);

// Two versions of switch_display_view:
void switch_display_view(lv_timer_t *timer); // For timer callback
void set_display_view(const char *command);  // For command-based control


#endif // LCD_DISPLAY_H