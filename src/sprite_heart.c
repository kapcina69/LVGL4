#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_SPRITE_HEART
#define LV_ATTRIBUTE_IMG_SPRITE_HEART
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_SPRITE_HEART uint8_t Sprite_heart_map[] = {
  0x00, 0x00, 
  0x00, 0x00, 
  0x3c, 0x3c, 
  0x7e, 0x7e, 
  0xff, 0xff, 
  0xff, 0xff, 
  0xff, 0xff, 
  0xff, 0xff, 
  0x7f, 0xfe, 
  0x3f, 0xfc, 
  0x1f, 0xf8, 
  0x0f, 0xf0, 
  0x07, 0xe0, 
  0x03, 0xc0, 
  0x01, 0x80, 
  0x00, 0x00, 
};

const lv_img_dsc_t Sprite_heart = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 16,
  .header.h = 16,
  .data_size = 32,
  .data = Sprite_heart_map,
};