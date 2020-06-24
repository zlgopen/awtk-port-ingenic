#include "base/g2d.h"
#include <pthread.h>

#include <stdio.h>
#include "jzhal.h"

typedef struct _graphic_buffer_jzgpu_t {
  graphic_buffer_t graphic_buffer;
  uint8_t* data;
  void* pwdata;
} graphic_buffer_jzgpu_t;

#define GRAPHIC_BUFFER_JZGPU(buffer) ((graphic_buffer_jzgpu_t*)(buffer))

#define _JHMEM_PTR(bitmap_ptr) (GRAPHIC_BUFFER_JZGPU(bitmap_ptr->buffer)->pwdata)
#define JHMEM_PTR(bitmap_ptr) (JH_MEM *)(_JHMEM_PTR(bitmap_ptr))

extern void linux_fb_pwdata_deal(bitmap_t *fb,bitmap_t* img);

extern simd2d_info_t * mxu;

ret_t g2d_fill_rect(bitmap_t* fb, rect_t* dst, color_t c)
{
    linux_fb_pwdata_deal(fb,NULL);
    color_t c_gpu = c;
    if(mxu->fill(JHMEM_PTR(fb),(JH_RECT*)dst, c_gpu.color)){
        return RET_OK;
    }else{
        return RET_NOT_IMPL;
    }
}

ret_t g2d_copy_image(bitmap_t* fb, bitmap_t* img, rect_t* src, xy_t x, xy_t y)
{
    linux_fb_pwdata_deal(fb,img);
    if(mxu->copy(JHMEM_PTR(img), JHMEM_PTR(fb), (JH_RECT* )src, (int32_t)x, (int32_t)y)== 0){
        return RET_OK;
    }else{
        return RET_NOT_IMPL;
    }
}

ret_t g2d_blend_image(bitmap_t* fb, bitmap_t* img, rect_t* dst, rect_t* src, uint8_t global_alpha)
{
    linux_fb_pwdata_deal(fb,img);
    if(mxu->blend(JHMEM_PTR(img), JHMEM_PTR(fb), (JH_RECT*) src, (JH_RECT*) dst, global_alpha) == 0){
        return RET_OK;
    }else{
        return RET_NOT_IMPL;
    }
}

ret_t g2d_rotate_image(bitmap_t* fb, bitmap_t* img, rect_t* src, lcd_orientation_t o)
{
    linux_fb_pwdata_deal(fb,img);
#ifdef ACCEL_X1830_MXU
    if(o == LCD_ORIENTATION_90)
        o = LCD_ORIENTATION_270;
    else if(o == LCD_ORIENTATION_270)
        o = LCD_ORIENTATION_90;
#endif

    if(mxu->rotate(JHMEM_PTR(img), JHMEM_PTR(fb), (JH_RECT*)src, (int)o) == 0)
        return RET_OK;
    else
        return RET_NOT_IMPL;
}
