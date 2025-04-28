#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <lvgl.h>

extern lv_obj_t *hr_label;
extern lv_obj_t *steps_label;

void init_lcd_display(void);
void update_lcd_display(uint32_t hr, uint32_t steps);
void switch_display_view(lv_timer_t *timer);





#endif // LCD_DISPLAY_H
