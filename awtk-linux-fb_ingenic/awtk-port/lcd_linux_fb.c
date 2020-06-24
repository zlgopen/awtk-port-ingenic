/**
 * File:   lcd_linux_fb.h
 * Author: AWTK Develop Team
 * Brief:  linux framebuffer lcd
 *
 * Copyright (c) 2018 - 2018  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-09-07 Li XianJing <xianjimli@hotmail.com> created
 * 2019-04-17 Gao Wei <wei.gao@ingenic.com> created
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "tkc/mem.h"
#include "tkc/utf8.h"
#include "lcd/lcd_mem_bgr565.h"
#include "lcd/lcd_mem_bgra8888.h"
#include "lcd/lcd_mem_rgba8888.h"
#include "blend/image_g2d.h"
#include "base/system_info.h"
#include "lcd_linux_fb.h"

#ifdef ACCEL_M200_BRILLO
#include "JzhalManager_api_C.h"
#endif

#include "jzhal.h"
#include "base/g2d.h"

#define JZHAL_ID "awtk-port"

#define FB_NUM 2
bitmap_t *bitmap_fb[FB_NUM+1]; // +1 for shadow.
#define SHADOW 2
int online_no;
int offline_no;

#ifdef ACCEL_X1830_MXU
simd2d_info_t * mxu;
#endif

gralloc_info_t * gralloc = NULL;
fb_info_t * fb = NULL;

rect_t rect_last;

typedef struct _graphic_buffer_jzgpu_t {
  graphic_buffer_t graphic_buffer;
  uint8_t* data;
  void* pwdata;
} graphic_buffer_jzgpu_t;

#define GRAPHIC_BUFFER_JZGPU(buffer) ((graphic_buffer_jzgpu_t*)(buffer))

#define _JHMEM_PTR(bitmap_ptr) (GRAPHIC_BUFFER_JZGPU(bitmap_ptr->buffer)->pwdata)
#define JHMEM_PTR(bitmap_ptr) (JH_MEM *)(_JHMEM_PTR(bitmap_ptr))

int convert(uint16_t format)
{
    //printf("\033[33m format = %d |\033[0m\n", format);
    switch(format) {
    case BITMAP_FMT_RGBA8888:
        return JH_FMT_RGBA8888;
    case BITMAP_FMT_BGRA8888:
        return JH_FMT_BGRA8888;
    case BITMAP_FMT_BGR565:
        return JH_FMT_BGR565;
    default:
#if 0
        return JH_FMT_ARGB8888;
        return JH_FMT_ABGR8888;
        return JH_FMT_RGB565;
        return JH_FMT_RGB888;
        return JH_FMT_BGR888;
        return JH_FMT_NV12;
        return JH_FMT_NV21;
        return JH_FMT_YUV420_TILE;
        return JH_FMT_YV12;
        return JH_FMT_I420;
        return JH_FMT_YUY2;
        return JH_FMT_UYVY;
        return JH_FMT_ALPHA;
#endif
        break;
    }
    //printf("\033[31m(l:%d, f:%s, F: %s) %d %s\033[0m\n", __LINE__, __func__, __FILE__, 0, "");
    return JH_FMT_NONE;
}

int jhmem_alloc_buffer(bitmap_t *bitmap_info)
{
    if (bitmap_info->buffer == NULL) {
        bitmap_info->buffer = calloc(1, sizeof(graphic_buffer_jzgpu_t));
    }
#ifdef ACCEL_M200_BRILLO
    JH_MEM *data = jzhal_alloc_mem(bitmap_info->w, bitmap_info->h, convert(bitmap_info->format),FALSE);
#else
    JH_MEM *data = gralloc->alloc_mem(bitmap_info->w, bitmap_info->h, convert(bitmap_info->format));
#endif
    _JHMEM_PTR(bitmap_info) = data;
    GRAPHIC_BUFFER_JZGPU(bitmap_info->buffer)->data = data->virt[0];
    bitmap_info->line_length = data->stride[0];
    /* jhmem_check_and_fix_gpu2d(data); */

    return 0;
}

int jhmem_free_buffer(graphic_buffer_jzgpu_t* bitmap_buffer)
{
    if (bitmap_buffer != NULL) {
#ifdef ACCEL_M200_BRILLO
        jzhal_free_mem((JH_MEM *)(bitmap_buffer->pwdata));
#else
        gralloc->free_mem((JH_MEM *)(bitmap_buffer->pwdata));
#endif
    }
    return 0;
}

void linux_fb_pwdata_deal(bitmap_t *fb,bitmap_t* img)
{
    if(get_linux_fb_offline()==NULL)
        return;
    graphic_buffer_jzgpu_t* offline = GRAPHIC_BUFFER_JZGPU(get_linux_fb_offline()->buffer);
    if(offline != NULL && offline->pwdata != NULL){
        if(fb != NULL){
            graphic_buffer_jzgpu_t* fb_buffer = GRAPHIC_BUFFER_JZGPU(fb->buffer);
            if(fb_buffer != NULL && fb_buffer->pwdata == NULL){
                fb_buffer->pwdata = offline->pwdata;
            }
        }
        if(img != NULL){
            graphic_buffer_jzgpu_t* img_buffer = GRAPHIC_BUFFER_JZGPU(img->buffer);
            if(img_buffer !=NULL && img_buffer->pwdata == NULL){
                img_buffer->pwdata = offline->pwdata;
            }
        }
    }
}
#if (defined ACCEL_M200_GPU2D)||(defined ACCEL_M200_GPU3D)
gpu_info_t * gpu = NULL;

#include "gpu.c"
#endif

#ifdef ACCEL_M200_BRILLO
#include "gpu_brillo.c"
#endif

#ifdef ACCEL_M200_GPU3D
#include "vgcanvas/lcd_vgcanvas_jz.inc"
void gpu_makeCurrent(void)
{
    if(gpu != NULL && gpu->gpu3d_function.makeCurrent != NULL)
        gpu->gpu3d_function.makeCurrent();
}
#endif

lcd_orientation_t orientation = LCD_ORIENTATION_0;
static int s_lcd_line_length_default;
static int merge_enable = 1;
void merge_set_enable(int enable)
{
    merge_enable = enable;
}

bitmap_t * get_linux_fb_offline(void)
{
#ifdef ACCEL_M200_GPU3D
    if(bitmap_fb[SHADOW] != NULL && GRAPHIC_BUFFER_JZGPU(bitmap_fb[SHADOW]->buffer)->pwdata != NULL)
#else
    if(bitmap_fb[SHADOW]!=NULL)
#endif
        return bitmap_fb[SHADOW];
    else
        return bitmap_fb[offline_no];
}

#if 0
static uint32_t lcd_mem_get_line_length(lcd_mem_t* mem)
{
    uint32_t bpp = 4;
    return tk_max(mem->base.w * bpp, mem->line_length);
}
#endif

static ret_t lcd_mem_draw_image(lcd_t* lcd, bitmap_t* img, rect_t* src, rect_t* dst)
{
    bitmap_t *offline;

    if(orientation != LCD_ORIENTATION_0)
	offline = bitmap_fb[SHADOW];
    else
	offline = bitmap_fb[offline_no];

    if(dst->y + src->h <= 0 || dst->x + src->w <= 0 ||
	    dst->y >= lcd->h || dst->x >= lcd->w){
	return RET_OK;
    }
    ret_t ret = RET_OK;
    bool_t is_opaque = (img->flags & BITMAP_FLAG_OPAQUE || img->format == BITMAP_FMT_RGB565);

    if (img->format == offline->format && is_opaque && src->w == dst->w && src->h == dst->h &&
	    lcd->global_alpha >= TK_OPACITY_ALPHA) {
	rect_t r;
	xy_t dx = 0;
	xy_t dy = 0;
	r = *src;
	dx = dst->x;
	dy = dst->y;

	ret = image_copy(offline, img, &r, dx, dy);
    } else {
	ret = image_blend(offline, img, dst, src, lcd->global_alpha);
    }

    return ret;
}

static ret_t lcd_mem_take_snapshot(lcd_t* lcd, bitmap_t* img, bool_t auto_rotate)
{
    bitmap_t *offline;

    if(orientation != LCD_ORIENTATION_0)
	offline = bitmap_fb[SHADOW];
    else
	offline = bitmap_fb[offline_no];

    rect_t r = rect_init(0, 0, offline->w, offline->h);
    return_value_if_fail(bitmap_init(img, offline->w, offline->h, (bitmap_format_t)(offline->format), NULL) == RET_OK,
            RET_OOM);

    return image_copy(img, offline, &r, 0, 0);
}

static ret_t lcd_linux_fb_resize(lcd_t* lcd, wh_t w, wh_t h, uint32_t line_length)
{
    lcd_mem_t* mem = (lcd_mem_t*)lcd;

    lcd->w = w;
    lcd->h = h;
    mem->line_length = s_lcd_line_length_default;

    orientation = system_info()->lcd_orientation;

    if(orientation != LCD_ORIENTATION_0){

        lcd_mem_t* mem = (lcd_mem_t*)lcd;
        bitmap_fb[SHADOW] = calloc(1, sizeof(bitmap_t));
        bitmap_init(bitmap_fb[SHADOW], w, h, mem->format, NULL);
#if (defined ACCEL_M200_GPU3D) || (defined ACCEL_M200_GPU2D) || (defined ACCEL_M200_BRILLO)
        jhmem_free_buffer(GRAPHIC_BUFFER_JZGPU(bitmap_fb[SHADOW]->buffer));
        jhmem_alloc_gpu_buffer(bitmap_fb[SHADOW]);
        // TODO:
        /* gralloc->free_mem(JHMEM_PTR(bitmap_fb[offline_no])); */
        /* gralloc->free_mem(JHMEM_PTR(bitmap_fb[online_no])); */
#endif

#ifdef ACCEL_M200_GPU3D
        set_vg_framebuffer(bitmap_fb[SHADOW]); /* swap vg framebuffer for shadow context. */
#endif

        mem->offline_fb = (uint8_t*)GRAPHIC_BUFFER_DEFAULT(bitmap_fb[SHADOW]->buffer)->data;
        mem->line_length = bitmap_fb[SHADOW]->line_length;
        vgcanvas_t* vg = lcd->get_vgcanvas(lcd);
        vg->w = bitmap_fb[SHADOW]->w;
        vg->h = bitmap_fb[SHADOW]->h;
    }

    return RET_OK;
}

static void get_true_rect(rect_t* rect,int w,int h,lcd_orientation_t o)
{
    rect_t oldrect = {rect->x,rect->y,rect->w,rect->h};
    if(o == LCD_ORIENTATION_90){
        rect->x = w-oldrect.y-oldrect.h;
        rect->y = oldrect.x;
        rect->w = oldrect.h;
        rect->h = oldrect.w;
    }else if(o == LCD_ORIENTATION_270){
        rect->x = oldrect.y;
        rect->y = h-oldrect.x-oldrect.w;
        rect->w = oldrect.h;
        rect->h = oldrect.w;
    }
}

static ret_t lcd_linux_fb_flush(lcd_t* lcd)
{
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    rect_t* rects = &(lcd->dirty_rect);

    /* 1 step: copy shadow buffer to offline'fb if exist. */
    if (orientation != LCD_ORIENTATION_0) {
        image_rotate(bitmap_fb[offline_no], bitmap_fb[SHADOW], rects, orientation);
        get_true_rect(rects,bitmap_fb[SHADOW]->h,bitmap_fb[SHADOW]->w,orientation);
    }

    /* 2 step: swap online-offline buffer. */
    offline_no = online_no;
    online_no  = (online_no+1)%FB_NUM;

    if (orientation == LCD_ORIENTATION_0){
        mem->offline_fb = (uint8_t*)GRAPHIC_BUFFER_DEFAULT(bitmap_fb[offline_no]->buffer)->data;
        mem->online_fb = (uint8_t*)GRAPHIC_BUFFER_DEFAULT(bitmap_fb[online_no]->buffer)->data;
#ifdef ACCEL_M200_GPU3D
        // TODO:
         set_vg_framebuffer(bitmap_fb[offline_no]); /* swap vg framebuffer for GL context. */
#else
	bitmap_t * off_fb = bitmap_fb[offline_no];
        vgcanvas_t* canvas = mem->vgcanvas;
        if(canvas == NULL)
            canvas = lcd_get_vgcanvas(lcd);
        canvas->vt->reinit(canvas, off_fb->w, off_fb->h, off_fb->line_length, off_fb->format, GRAPHIC_BUFFER_DEFAULT(off_fb->buffer)->data);
#endif
	}

    fb->pan_display(0, online_no);
    fb->fb_sync();

    /* 3 step: merge buffer. */
    if(merge_enable){
        if (rect_last.w != rects->w
                || rect_last.h != rects->h
                || rect_last.x != rects->x
                || rect_last.y != rects->y)
#if (defined ACCEL_M200_GPU2D)||(defined ACCEL_M200_GPU3D)||(defined ACCEL_M200_BRILLO)
            gpu_fb_merge(bitmap_fb[online_no], bitmap_fb[offline_no], rects);
#else
        {
            image_copy(bitmap_fb[offline_no],
                    bitmap_fb[online_no],
                    rects, rects->x, rects->y);
        }
#endif
        memcpy(rects, &rect_last, sizeof(rect_t));
    }
    return RET_OK;
}

static void on_app_exit(void) {
    if(bitmap_fb[SHADOW] != NULL)
    {
        bitmap_destroy(bitmap_fb[SHADOW]);
        free(bitmap_fb[SHADOW]);
    }
    jzhal_gralloc_close(JZHAL_ID);
}

static ret_t lcd_mem_fill_rect_with_color(lcd_t* lcd, xy_t x, xy_t y, wh_t w, wh_t h, color_t c)
{
    bitmap_t *fb;
    rect_t r = rect_init(x, y, w, h);
    c.rgba.a = (c.rgba.a * lcd->global_alpha) / 0xff;

    fb = get_linux_fb_offline();
    return image_fill(fb, &r, c);
}

static ret_t lcd_mem_fill_rect(lcd_t* lcd, xy_t x, xy_t y, wh_t w, wh_t h)
{
  return lcd_mem_fill_rect_with_color(lcd, x, y, w, h, lcd->fill_color);
}

lcd_t* lcd_linux_fb_create(const char* filename)
{
    lcd_t* lcd = NULL;

    return_value_if_fail(filename != NULL, NULL);

    gralloc = jzhal_gralloc_open(JZHAL_ID);
    fb = jzhal_fb_open(JZHAL_ID);
    fb->enable_vsync(1);

    /* construct lcd_t */
    if (fb) {
	int i;
	int w = fb->var.xres;
	int h = fb->var.yres;
	int line_length = fb->fix.line_length;
	int bpp = fb->var.bits_per_pixel;
        bitmap_format_t format;
#ifdef ACCEL_M200_GPU2D
        gpu = jzhal_gpu2d_open(JZHAL_ID);
#endif
#ifdef ACCEL_M200_GPU3D
        gpu = jzhal_gpu2d_open(JZHAL_ID);
        gpu = jzhal_gpu3d_open(JZHAL_ID);
#endif
#ifdef ACCEL_X1830_MXU
        mxu = jzhal_simd2d_open(JZHAL_ID);
#endif
        if (bpp == 16) {
            format = BITMAP_FMT_BGR565;
        } else if (bpp == 32) {
            if (fb->var.blue.offset == 0) {
                format = BITMAP_FMT_BGRA8888;
            } else {
                format = BITMAP_FMT_RGBA8888;
            }
        } else {
            assert(!"not supported framebuffer format.");
        }

	for (i = 0; i < FB_NUM; ++i) {
            JH_MEM *fb_buf = fb->wrap_framebuffer(i);
	    // wrap bitmap_t for AWTK.
	    bitmap_fb[i] = calloc(1, sizeof(bitmap_t));

	    bitmap_init(bitmap_fb[i], w, h, format, fb_buf->virt[0]);
	    bitmap_set_line_length(bitmap_fb[i], line_length);

        graphic_buffer_jzgpu_t* bitmap_buffer = GRAPHIC_BUFFER_JZGPU(bitmap_fb[i]->buffer);

        bitmap_buffer->pwdata = (void*)fb_buf;
        bitmap_buffer->data = fb_buf->virt[0];
	}

	online_no = 0;
	offline_no = 1;

#ifdef ACCEL_M200_GPU3D
        // TODO:
	 set_vg_framebuffer(bitmap_fb[offline_no]);
#endif

	if (bpp == 16) {
	    lcd = lcd_mem_bgr565_create_double_fb(w, h,
		    (uint8_t*)GRAPHIC_BUFFER_DEFAULT(bitmap_fb[online_no]->buffer)->data,
		    (uint8_t*)GRAPHIC_BUFFER_DEFAULT(bitmap_fb[offline_no]->buffer)->data);
	} else if (bpp == 32) {
	    if (fb->var.blue.offset == 0) {
		lcd = lcd_mem_bgra8888_create_double_fb(w, h,
			(uint8_t*)GRAPHIC_BUFFER_DEFAULT(bitmap_fb[online_no]->buffer)->data,
			(uint8_t*)GRAPHIC_BUFFER_DEFAULT(bitmap_fb[offline_no]->buffer)->data);
	    } else {
		lcd = lcd_mem_rgba8888_create_double_fb(w, h,
			(uint8_t*)GRAPHIC_BUFFER_DEFAULT(bitmap_fb[online_no]->buffer)->data,
			(uint8_t*)GRAPHIC_BUFFER_DEFAULT(bitmap_fb[offline_no]->buffer)->data);
	    }
	} else {
	    assert(!"not supported framebuffer format.");
	}

	lcd_mem_set_line_length(lcd, line_length);
	s_lcd_line_length_default = line_length;

	if (lcd != NULL) {
	    lcd->flush = lcd_linux_fb_flush;
	    lcd->take_snapshot = lcd_mem_take_snapshot;
	    lcd->support_dirty_rect = TRUE;
	    lcd->resize = lcd_linux_fb_resize;
#if (defined ACCEL_X1830_MXU)||(defined ACCEL_M200_GPU2D)||(defined ACCEL_M200_BRILLO)
	    lcd->fill_rect = lcd_mem_fill_rect;
#endif
#ifdef ACCEL_M200_GPU3D
        lcd_mem_t* lcd_mem = (lcd_mem_t *)lcd;
	    lcd_mem->vgcanvas = vgcanvas_create_jz_gl(w, h,
		    lcd_mem->line_length, (bitmap_format_t)(lcd_mem->format),
		    (uint32_t*)(lcd_mem->offline_fb));

        lcd->begin_frame = lcd_mem_begin_frame;//
        lcd->set_clip_rect = lcd_vgcanvas_set_clip_rect;//
        lcd->get_clip_rect = lcd_vgcanvas_get_clip_rect;//
        lcd->draw_vline = lcd_vgcanvas_draw_vline;
        lcd->draw_hline = lcd_vgcanvas_draw_hline;
        lcd->fill_rect = lcd_vgcanvas_fill_rect;
        lcd->stroke_rect = lcd_vgcanvas_stroke_rect;
        lcd->draw_image = lcd_vgcanvas_draw_image;
        lcd->draw_image_matrix = lcd_vgcanvas_draw_image_matrix;
        lcd->draw_points = lcd_vgcanvas_draw_points;
        lcd->draw_text = lcd_vgcanvas_draw_text;//
        lcd->measure_text = lcd_vgcanvas_measure_text;//
        lcd->end_frame = lcd_mem_end_frame;//
        lcd->get_vgcanvas = lcd_vgcanvas_get_vgcanvas;//
        lcd->set_font_name = lcd_vgcanvas_set_font_name;//
        lcd->set_font_size = lcd_vgcanvas_set_font_size;//
        lcd->set_global_alpha = lcd_vgcanvas_set_global_alpha;//
        lcd->get_desired_bitmap_format = lcd_vgcanvas_get_desired_bitmap_format;
        lcd->get_width = lcd_vgcanvas_get_width;
        lcd->get_height = lcd_vgcanvas_get_height;
        lcd->destroy = lcd_vgcanvas_destroy;
#else
	    lcd->draw_image = lcd_mem_draw_image;
#endif
	}
    }

    atexit(on_app_exit);

    return lcd;
}
