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
 * 2019-05-07 David.G <wei.gao@ingenic.com> modified
 *
 */

#ifndef TK_LCD_LINUX_FB_H
#define TK_LCD_LINUX_FB_H

#include "base/lcd.h"

BEGIN_C_DECLS

extern bitmap_t * get_linux_fb_offline(void);
extern lcd_t* lcd_linux_fb_create(const char* filename);

typedef struct _graphic_buffer_default_t {
  graphic_buffer_t graphic_buffer;

  uint8_t* data;
  uint8_t* data_head;
} graphic_buffer_default_t;

#define GRAPHIC_BUFFER_DEFAULT(buffer) ((graphic_buffer_default_t*)(buffer))

END_C_DECLS

#endif /*TK_LCD_LINUX_FB_H*/
