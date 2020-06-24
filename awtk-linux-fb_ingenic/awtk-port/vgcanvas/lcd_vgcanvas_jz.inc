static ret_t lcd_vgcanvas_set_font_name(lcd_t* lcd, const char* name) {
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;
    return vgcanvas_set_font(canvas, name);
}

static ret_t lcd_vgcanvas_set_font_size(lcd_t* lcd, uint32_t size) {
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;
    return vgcanvas_set_font_size(canvas, size);
}

static ret_t lcd_mem_begin_frame(lcd_t* lcd, rect_t* dirty_rect)
{
    lcd_mem_t* mem = (lcd_mem_t*)lcd;

    return vgcanvas_begin_frame(mem->vgcanvas, dirty_rect);
}

static ret_t lcd_vgcanvas_set_clip_rect(lcd_t* lcd, rect_t* r) {
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;
    return vgcanvas_clip_rect(canvas, r->x, r->y, r->w, r->h);
}

static ret_t lcd_vgcanvas_get_clip_rect(lcd_t* lcd, rect_t* r) {
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;
    *r = canvas->clip_rect;

    return RET_OK;
}

static ret_t lcd_vgcanvas_set_global_alpha(lcd_t* lcd, uint8_t alpha) {
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;
    return vgcanvas_set_global_alpha(canvas, (float_t)alpha / 255.0);
}


static ret_t lcd_vgcanvas_draw_vline(lcd_t* lcd, xy_t x, xy_t y, wh_t h) {
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;

    vgcanvas_begin_path(canvas);
    vgcanvas_move_to(canvas, x + 0.5f, y);
    vgcanvas_line_to(canvas, x + 0.5f, y + h);
    vgcanvas_set_line_width(canvas, 1);
    vgcanvas_set_stroke_color(canvas, lcd->stroke_color);
    vgcanvas_stroke(canvas);

    return RET_OK;
}

static ret_t lcd_vgcanvas_draw_hline(lcd_t* lcd, xy_t x, xy_t y, wh_t w) {
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;

    vgcanvas_begin_path(canvas);
    vgcanvas_move_to(canvas, x, y + 0.5f);
    vgcanvas_line_to(canvas, x + w, y + 0.5f);
    vgcanvas_set_line_width(canvas, 1);
    vgcanvas_set_stroke_color(canvas, lcd->stroke_color);
    vgcanvas_stroke(canvas);

    return RET_OK;
}

static ret_t lcd_vgcanvas_fill_rect(lcd_t* lcd, xy_t x, xy_t y, wh_t w, wh_t h) {
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;

  if (w > 1 && h > 1) {
    vgcanvas_begin_path(canvas);
    vgcanvas_set_antialias(canvas, FALSE);
    vgcanvas_rect(canvas, x, y, w, h);
    vgcanvas_set_fill_color(canvas, lcd->fill_color);
    vgcanvas_fill(canvas);
    vgcanvas_set_antialias(canvas, TRUE);
  } else if (w <= 1) {
    vgcanvas_begin_path(canvas);
    vgcanvas_move_to(canvas, x + 0.5f, y);
    vgcanvas_line_to(canvas, x + 0.5f, y + h);
    vgcanvas_set_line_width(canvas, 1);
    vgcanvas_set_stroke_color(canvas, lcd->fill_color);
    vgcanvas_stroke(canvas);
  } else if (h <= 1) {
    vgcanvas_begin_path(canvas);
    vgcanvas_move_to(canvas, x, y + 0.5f);
    vgcanvas_line_to(canvas, x + w, y + 0.5f);
    vgcanvas_set_line_width(canvas, 1);
    vgcanvas_set_stroke_color(canvas, lcd->fill_color);
    vgcanvas_stroke(canvas);
  }

    return RET_OK;
}

static ret_t lcd_vgcanvas_stroke_rect(lcd_t* lcd, xy_t x, xy_t y, wh_t w, wh_t h) {
  float_t offset_1 = 1.0f / lcd->ratio;
  float_t offset_5 = 0.5f / lcd->ratio;
  lcd_mem_t* mem = (lcd_mem_t*)lcd;
  vgcanvas_t* canvas = mem->vgcanvas;

  vgcanvas_begin_path(canvas);
  vgcanvas_rect(canvas, x + offset_5, y + offset_5, w - offset_1, h - offset_1);
  vgcanvas_set_line_width(canvas, 1);
  vgcanvas_set_stroke_color(canvas, lcd->stroke_color);
  vgcanvas_stroke(canvas);

  return RET_OK;
}

static ret_t lcd_vgcanvas_draw_points(lcd_t* lcd, point_t* points, uint32_t nr) {
    uint32_t i = 0;
    float_t offset = 0.5f * lcd->ratio;
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;
    vgcanvas_set_stroke_color(canvas, lcd->stroke_color);
  vgcanvas_set_line_width(canvas, 1);
  for (i = 0; i < nr; i++) {
    float x = points[i].x;
    float y = points[i].y;

    x += offset;

    vgcanvas_begin_path(canvas);
    vgcanvas_move_to(canvas, x, y + offset);
    vgcanvas_line_to(canvas, x, y);
    vgcanvas_stroke(canvas);
  }

  return RET_OK;
}

static ret_t lcd_vgcanvas_draw_image(lcd_t* lcd, bitmap_t* img, rect_t* src, rect_t* dst) {
    int sx = src->x;
    int sy = src->y;
    int sw = src->w;
    int sh = src->h;
    int dx = dst->x;
    int dy = dst->y;
    int dw = dst->w;
    int dh = dst->h;
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;

    return vgcanvas_draw_image(canvas, img, sx, sy, sw, sh, dx, dy, dw, dh);
}

static ret_t lcd_vgcanvas_draw_image_matrix(lcd_t* lcd, draw_image_info_t* info) {
    matrix_t* m = &(info->matrix);
    rect_t* s = &(info->src);
    rect_t* d = &(info->dst);
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;
    vgcanvas_save(canvas);
    vgcanvas_set_transform(canvas, m->a0, m->a1, m->a2, m->a3, m->a4, m->a5);
    vgcanvas_draw_image(canvas, info->img, s->x, s->y, s->w, s->h, d->x, d->y, d->w, d->h);
    vgcanvas_restore(canvas);

    return RET_OK;
}

#define MAX_CHARS 256
static float_t lcd_vgcanvas_measure_text(lcd_t* lcd, const wchar_t* str, uint32_t nr) {
    wchar_t temp[MAX_CHARS + 1];
    char text[2 * MAX_CHARS + 1];
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;
    return_value_if_fail(nr < MAX_CHARS, RET_FAIL);

    if (nr == 0) {
        return 0;
    }

    wcsncpy(temp, str, nr);
    temp[nr] = 0;

    tk_utf8_from_utf16(temp, text, sizeof(text) - 1);

    vgcanvas_set_font(canvas, lcd->font_name);
    vgcanvas_set_font_size(canvas, lcd->font_size);
    return vgcanvas_measure_text(canvas, text);
}

static ret_t lcd_vgcanvas_draw_text(lcd_t* lcd, const wchar_t* str, uint32_t nr, xy_t x, xy_t y)
{
    wchar_t temp[MAX_CHARS + 1];
    char text[2 * MAX_CHARS + 1];
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;
    return_value_if_fail(nr < MAX_CHARS, RET_FAIL);

    if (nr == 0) {
        return RET_OK;
    }

    wcsncpy(temp, str, nr);
    temp[nr] = 0;

    tk_utf8_from_utf16(temp, text, sizeof(text) - 1);

    vgcanvas_set_font(canvas, lcd->font_name);
    vgcanvas_set_font_size(canvas, lcd->font_size);
    vgcanvas_set_fill_color(canvas, lcd->text_color);
    vgcanvas_set_text_align(canvas, "left");
    vgcanvas_set_text_baseline(canvas, "top");

    return vgcanvas_fill_text(canvas, text, x, y, 0xffff);
}

static wh_t lcd_vgcanvas_get_width(lcd_t* lcd) {
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;

    return vgcanvas_get_width(canvas);
}

static wh_t lcd_vgcanvas_get_height(lcd_t* lcd) {
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;

    return vgcanvas_get_height(canvas);
}

static ret_t lcd_mem_end_frame(lcd_t* lcd)
{
    lcd_mem_t* mem = (lcd_mem_t*)lcd;

    if (mem->vgcanvas) {
        vgcanvas_end_frame(mem->vgcanvas);
        glFinish();
    }
    if (lcd->draw_mode == LCD_DRAW_OFFLINE) {
        return RET_OK;
    }

    return lcd_flush(lcd);
}

static ret_t lcd_vgcanvas_destroy(lcd_t* lcd) {
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;
    vgcanvas_destroy(canvas);
    memset(lcd, 0x00, sizeof(lcd_t));
    TKMEM_FREE(lcd);

    return RET_OK;
}

static vgcanvas_t* lcd_vgcanvas_get_vgcanvas(lcd_t* lcd) {
    lcd_mem_t* mem = (lcd_mem_t*)lcd;
    vgcanvas_t* canvas = mem->vgcanvas;
    //vgcanvas_begin_path(canvas);
    return canvas;
}

static bitmap_format_t lcd_vgcanvas_get_desired_bitmap_format(lcd_t* lcd) {
  return BITMAP_FMT_RGBA8888;
}

