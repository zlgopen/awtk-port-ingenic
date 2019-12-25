/**
 * File:   text_hl_selector.h
 * Author: AWTK Develop Team
 * Brief:  text_hl_selector
 *
 * Copyright (c) 2018 - 2019  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2018-09-25 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utf8.h"
#include "tkc/utils.h"
#include "tkc/easing.h"
#include "widgets/button.h"
#include "base/layout.h"
#include "widgets/popup.h"
#include "base/window.h"
#include "tkc/tokenizer.h"
#include "base/widget_factory.h"
#include "text_hl_selector/text_hl_selector.h"
#include "widget_animators/widget_animator_scroll.h"

const char* s_text_hl_selector_properties[] = {
    WIDGET_PROP_TEXT,           WIDGET_PROP_VALUE,
    WIDGET_PROP_OPTIONS,        TEXT_HL_SELECTOR_PROP_VISIBLE_NR,
    WIDGET_PROP_SELECTED_INDEX, NULL};

static ret_t text_hl_selector_paint_mask(widget_t* widget, canvas_t* c) {
  int32_t i = 0;
  int32_t y = 0;
  int32_t n = widget->h / 2;
  style_t* style = widget->astyle;
  color_t trans = color_init(0, 0, 0, 0);
  easing_func_t easing = easing_get(EASING_CUBIC_IN);
  color_t mask_color = style_get_color(style, STYLE_ID_MASK_COLOR, trans);

  if (mask_color.rgba.a) {
    for (i = 0; i < n; i++) {
      y = n - i - 1;

      mask_color.rgba.a = 0xff * easing((float_t)i / (float_t)n);
      canvas_set_stroke_color(c, mask_color);
      canvas_draw_hline(c, 0, y, widget->w);
    }

    for (i = 0; i < n; i++) {
      y = widget->h - n + i;

      mask_color.rgba.a = 0xff * easing((float_t)i / (float_t)n);
      canvas_set_stroke_color(c, mask_color);
      canvas_draw_hline(c, 0, y, widget->w);
    }
  }

  return RET_OK;
}

static ret_t text_hl_selector_paint_self(widget_t* widget, canvas_t* c) {
  text_hl_selector_option_t* iter = NULL;
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(widget != NULL && text_hl_selector != NULL, RET_BAD_PARAMS);

  uint32_t y = 0;
  uint32_t i = 0;
  style_t* style = widget->astyle;
  color_t trans = color_init(0, 0, 0, 0);
  int32_t yoffset = text_hl_selector->yoffset;
  int32_t visible_nr = text_hl_selector->visible_nr;
  int32_t item_height = widget->h / text_hl_selector->visible_nr;
  color_t fc = style_get_color(style, STYLE_ID_FG_COLOR, trans);
  rect_t r = rect_init(0, 0, widget->w, item_height);

  int32_t options_nr = text_hl_selector_count_options(widget);
  //int32_t empty_item_height = (text_hl_selector->visible_nr / 2) * item_height;
  //int32_t min_yoffset = -empty_item_height;
  //int32_t max_yoffset = (options_nr * item_height + empty_item_height) - widget->h;
  int32_t min_yoffset = (text_hl_selector->visible_nr - options_nr ) * item_height;
  int32_t max_yoffset = options_nr * item_height;

  bool_t loop = text_hl_selector->loop;
  if(!loop){
      text_hl_selector->loop = loop = style_get_int(style,SELECTOR_LOOP_MODE, 0);
  }

  widget_prepare_text_style(widget, c);
  iter = text_hl_selector->option_items;

  if (fc.rgba.a) {
    canvas_set_stroke_color(c, fc);
    for (i = 1; i < visible_nr; i++) {
      y = i * item_height;
      canvas_draw_hline(c, 0, y, widget->w);
    }
    y = 0;
    i = 0;
  }

  color_t color_normal = c->lcd->text_color;
  color_t color_select = style_get_color(style, STYLE_ID_SELECTED_TEXT_COLOR, trans);
  int item_pos;
  while (iter != NULL) {
    item_pos = y - yoffset;
    if(loop){
        if(item_pos > max_yoffset - item_height){
            item_pos = item_pos - max_yoffset;
        }else if(item_pos < min_yoffset + item_height){
            item_pos = visible_nr * item_height + item_pos - min_yoffset;
        }
    }
    //r.y = y - yoffset;
    r.y = item_pos;
    if ((r.y + item_height) >= 0 && r.y < widget->h) {
        if(color_select.rgba.a && (r.y + item_height) > widget->h/2 && r.y < widget->h/2 )
            canvas_set_text_color(c, color_select);
      canvas_draw_text_in_rect(c, iter->text, wcslen(iter->text), &r);
        if(color_select.rgba.a && (r.y + item_height) > widget->h/2 && r.y < widget->h/2 )
            canvas_set_text_color(c, color_normal);
    }

    i++;
    iter = iter->next;
    y += item_height;
  }

  return RET_OK;
}

static ret_t text_hl_selector_on_paint_self(widget_t* widget, canvas_t* c) {
  rect_t r_save;
  rect_t r = rect_init(c->ox, c->oy, widget->w, widget->h);
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  canvas_get_clip_rect(c, &r_save);
  r = rect_intersect(&r_save, &r);
  if (r.w > 0 && r.h > 0) {
    canvas_set_clip_rect(c, &r);
    text_hl_selector_paint_self(widget, c);
    text_hl_selector_paint_mask(widget, c);
    canvas_set_clip_rect(c, &r_save);
  }

  return RET_OK;
}

static ret_t text_hl_selector_on_destroy(widget_t* widget) {
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(widget != NULL && text_hl_selector != NULL, RET_BAD_PARAMS);

  str_reset(&(text_hl_selector->text));
  text_hl_selector_reset_options(widget);

  return RET_OK;
}

ret_t text_hl_selector_parse_options(widget_t* widget, const char* str) {
  int32_t i = 0;
  tokenizer_t tokenizer;
  tokenizer_t* t = &tokenizer;
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(widget != NULL && text_hl_selector != NULL, RET_BAD_PARAMS);

  text_hl_selector_reset_options(widget);
  text_hl_selector->options = tk_strdup(str);
  tokenizer_init(t, str, strlen(str), ";");

  while (tokenizer_has_more(t)) {
    int32_t value = 0;
    const char* token = tokenizer_next(t);
    if (*token) {
      const char* text = strchr(token, ':');
      if (text != NULL) {
        text++;
        value = tk_atoi(token);
      } else {
        text = token;
        value = i;
      }

      text_hl_selector_append_option(widget, value, text);
      i++;
    }
  }
  tokenizer_deinit(t);

  return RET_OK;
}

static ret_t text_hl_selector_set_range_options_ex(widget_t* widget, int32_t start, uint32_t nr,
                                                int32_t step, const char* format) {
  char text[64];
  uint32_t i = 0;
  return_value_if_fail(widget != NULL && nr < 300 && format != NULL, RET_BAD_PARAMS);

  for (i = 0; i < nr; i++) {
    int32_t value = start + i * step;
    tk_snprintf(text, sizeof(text) - 1, format, value);
    text_hl_selector_append_option(widget, value, text);
  }

  return RET_OK;
}

ret_t text_hl_selector_set_range_options(widget_t* widget, int32_t start, uint32_t nr, int32_t step) {
  return text_hl_selector_set_range_options_ex(widget, start, nr, step, "%d");
}

ret_t text_hl_selector_set_options(widget_t* widget, const char* options) {
  return_value_if_fail(widget != NULL && options != NULL, RET_BAD_PARAMS);

  if (strchr(options, ':') == NULL && strchr(options, '-') != NULL) {
    int nr = 0;
    int end = 0;
    int start = 0;
    char format[32];
    memset(format, 0x00, sizeof(format));

    nr = tk_sscanf(options, "%d-%d-%31s", &start, &end, format);

    if (nr < 3) {
      tk_strncpy(format, "%d", sizeof(format) - 1);
    }

    return text_hl_selector_set_range_options_ex(widget, start, end - start + 1, 1, format);
  } else {
    return text_hl_selector_parse_options(widget, options);
  }
}

static ret_t text_hl_selector_get_prop(widget_t* widget, const char* name, value_t* v) {
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(widget != NULL && text_hl_selector != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_TEXT)) {
    value_set_str(v, text_hl_selector_get_text(widget));
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_VALUE)) {
    value_set_int(v, text_hl_selector_get_value(widget));
    return RET_OK;
  } else if (tk_str_eq(name, TEXT_HL_SELECTOR_PROP_VISIBLE_NR)) {
    value_set_int(v, text_hl_selector->visible_nr);
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_SELECTED_INDEX)) {
    value_set_int(v, text_hl_selector->selected_index);
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_XOFFSET)) {
    value_set_int(v, 0);
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_YOFFSET)) {
    value_set_int(v, text_hl_selector->yoffset);
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_OPTIONS)) {
    value_set_str(v, text_hl_selector->options);
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static ret_t text_hl_selector_set_prop(widget_t* widget, const char* name, const value_t* v) {
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(widget != NULL && text_hl_selector != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_VALUE)) {
    text_hl_selector_set_value(widget, value_int(v));
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_TEXT)) {
    text_hl_selector_set_text(widget, value_str(v));
    return RET_OK;
  } else if (tk_str_eq(name, TEXT_HL_SELECTOR_PROP_VISIBLE_NR)) {
    text_hl_selector_set_visible_nr(widget, value_int(v));
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_SELECTED_INDEX)) {
    text_hl_selector_set_selected_index(widget, value_int(v));
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_YOFFSET)) {
    text_hl_selector->yoffset = value_int(v);
    if(text_hl_selector->loop)
    {
        int32_t options_nr = text_hl_selector_count_options(widget);
        int32_t item_height = widget->h / text_hl_selector->visible_nr;
        //int32_t empty_item_height = (text_hl_selector->visible_nr / 2) * item_height;
        int32_t max_yoffset = (options_nr * item_height);
        //int32_t max_yoffset = (options_nr * item_height + empty_item_height) - widget->h;
        if(text_hl_selector->yoffset < 0){
            text_hl_selector->yoffset = max_yoffset + text_hl_selector->yoffset%max_yoffset;
        }else if(text_hl_selector->yoffset > max_yoffset){
            text_hl_selector->yoffset = text_hl_selector->yoffset%max_yoffset;
        }
    }
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_XOFFSET)) {
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_OPTIONS)) {
    text_hl_selector_set_options(widget, value_str(v));
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static ret_t text_hl_selector_on_pointer_down(text_hl_selector_t* text_hl_selector, pointer_event_t* e) {
  velocity_t* v = &(text_hl_selector->velocity);

  velocity_reset(v);
  text_hl_selector->ydown = e->y;
  text_hl_selector->yoffset_save = text_hl_selector->yoffset;

  velocity_update(v, e->e.time, e->x, e->y);

  return RET_OK;
}
static ret_t text_hl_selector_on_pointer_move(text_hl_selector_t* text_hl_selector, pointer_event_t* e) {
  velocity_t* v = &(text_hl_selector->velocity);
  int32_t dy = e->y - text_hl_selector->ydown;
  velocity_update(v, e->e.time, e->x, e->y);

  if (text_hl_selector->wa == NULL && dy) {
    text_hl_selector->yoffset = text_hl_selector->yoffset_save - dy;
    if(text_hl_selector->loop)
    {
        widget_t * widget = WIDGET(text_hl_selector);
        int32_t options_nr = text_hl_selector_count_options(widget);
        int32_t item_height = widget->h / text_hl_selector->visible_nr;
        //int32_t empty_item_height = (text_hl_selector->visible_nr / 2) * item_height;
        int32_t max_yoffset = (options_nr * item_height);
        //int32_t max_yoffset = (options_nr * item_height + empty_item_height) - widget->h;
        if(text_hl_selector->yoffset < 0){
            text_hl_selector->yoffset = max_yoffset + text_hl_selector->yoffset%max_yoffset;
        }else if(text_hl_selector->yoffset > max_yoffset){
            text_hl_selector->yoffset = text_hl_selector->yoffset%max_yoffset;
        }
    }
  }

  return RET_OK;
}

static ret_t text_hl_selector_set_selected_index_only(text_hl_selector_t* text_hl_selector, int32_t index) {
  widget_t* widget = WIDGET(text_hl_selector);
  return_value_if_fail(widget != NULL && text_hl_selector != NULL, RET_BAD_PARAMS);

  if (index != text_hl_selector->selected_index) {
    event_t e = event_init(EVT_VALUE_WILL_CHANGE, widget);
    widget_dispatch(widget, &e);
    text_hl_selector->selected_index = index;
    e = event_init(EVT_VALUE_CHANGED, widget);
    widget_dispatch(widget, &e);
  }

  widget_invalidate(widget, NULL);

  return RET_OK;
}

static ret_t text_hl_selector_sync_selected_index_with_yoffset(text_hl_selector_t* text_hl_selector) {
    widget_t* widget = WIDGET(text_hl_selector);
    int32_t mid_index = text_hl_selector->visible_nr / 2;
    int32_t item_height = widget->h / text_hl_selector->visible_nr;
    int32_t selected_index = text_hl_selector->yoffset / item_height + mid_index;
    int32_t options_nr = text_hl_selector_count_options(widget);
    if(selected_index >= options_nr)
        selected_index -= options_nr;
    return text_hl_selector_set_selected_index_only(text_hl_selector, selected_index);
}

static ret_t text_hl_selector_sync_yoffset_with_selected_index(text_hl_selector_t* text_hl_selector) {
  widget_t* widget = WIDGET(text_hl_selector);
  int32_t mid_index = text_hl_selector->visible_nr / 2;
  int32_t item_height = widget->h / text_hl_selector->visible_nr;

  text_hl_selector->yoffset = (text_hl_selector->selected_index - mid_index) * item_height;

  widget_invalidate(widget, NULL);

  return RET_OK;
}

static ret_t text_hl_selector_on_scroll_done(void* ctx, event_t* e) {
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(ctx);
  return_value_if_fail(text_hl_selector != NULL, RET_BAD_PARAMS);

  text_hl_selector->wa = NULL;
  text_hl_selector_sync_selected_index_with_yoffset(text_hl_selector);

  return RET_REMOVE;
}

static ret_t text_hl_selector_scroll_to(widget_t* widget, int32_t yoffset_end) {
  int32_t yoffset = 0;
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL, RET_FAIL);

  yoffset = text_hl_selector->yoffset;
  if (yoffset == yoffset_end) {
    return RET_OK;
  }

  text_hl_selector->wa = widget_animator_scroll_create(widget, TK_ANIMATING_TIME, 0, EASING_SIN_INOUT);
  return_value_if_fail(text_hl_selector->wa != NULL, RET_OOM);

  widget_animator_scroll_set_params(text_hl_selector->wa, 0, yoffset, 0, yoffset_end);
  widget_animator_on(text_hl_selector->wa, EVT_ANIM_END, text_hl_selector_on_scroll_done, text_hl_selector);
  widget_animator_start(text_hl_selector->wa);

  return RET_OK;
}

static ret_t text_hl_selector_on_pointer_up(text_hl_selector_t* text_hl_selector, pointer_event_t* e) {
  int32_t yoffset_end = 0;
  widget_t* widget = WIDGET(text_hl_selector);
  velocity_t* v = &(text_hl_selector->velocity);
  int32_t options_nr = text_hl_selector_count_options(widget);
  int32_t item_height = widget->h / text_hl_selector->visible_nr;
  int32_t empty_item_height = (text_hl_selector->visible_nr / 2) * item_height;
  int32_t min_yoffset = -empty_item_height;
  int32_t max_yoffset = (options_nr * item_height + empty_item_height) - widget->h;

  velocity_update(v, e->e.time, e->x, e->y);
  yoffset_end = text_hl_selector->yoffset - v->yv;

  if (e->y == text_hl_selector->ydown) {
    /*click*/
    int32_t index = 0;
    int32_t mid_index = text_hl_selector->visible_nr / 2;
    point_t p = {e->x, e->y};
    widget_to_local(widget, &p);
    index = p.y / item_height;
    if (index == mid_index) {
      return RET_OK;
    } else {
      yoffset_end = text_hl_selector->yoffset + item_height * (index - mid_index);
    }
  }

  if(!text_hl_selector->loop){
      if (yoffset_end < min_yoffset) {
          yoffset_end = min_yoffset;
      }

      if (yoffset_end > (max_yoffset)) {
          yoffset_end = max_yoffset;
      }
  }

  yoffset_end = tk_roundi((float)yoffset_end / (float)item_height) * item_height;
  text_hl_selector_scroll_to(widget, yoffset_end);

  return RET_OK;
}

static ret_t text_hl_selector_on_event(widget_t* widget, event_t* e) {
  uint16_t type = e->type;
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);

  switch (type) {
    case EVT_POINTER_DOWN:
      text_hl_selector->pressed = TRUE;
      widget_grab(widget->parent, widget);
      text_hl_selector_on_pointer_down(text_hl_selector, (pointer_event_t*)e);
      break;
    case EVT_POINTER_UP: {
      text_hl_selector->pressed = FALSE;
      text_hl_selector_on_pointer_up(text_hl_selector, (pointer_event_t*)e);
      widget_ungrab(widget->parent, widget);
      break;
    }
    case EVT_POINTER_MOVE: {
      pointer_event_t* evt = (pointer_event_t*)e;
      if (evt->pressed && text_hl_selector->pressed) {
        text_hl_selector_on_pointer_move(text_hl_selector, evt);
        widget_invalidate(widget, NULL);
      }
      break;
    }
    default:
      break;
  }

  return RET_OK;
}

TK_DECL_VTABLE(text_hl_selector) = {.size = sizeof(text_hl_selector_t),
                                 .inputable = TRUE,
                                 .type = WIDGET_TYPE_TEXT_HL_SELECTOR,
                                 .clone_properties = s_text_hl_selector_properties,
                                 .persistent_properties = s_text_hl_selector_properties,
                                 .parent = TK_PARENT_VTABLE(widget),
                                 .create = text_hl_selector_create,
                                 .on_paint_self = text_hl_selector_on_paint_self,
                                 .set_prop = text_hl_selector_set_prop,
                                 .get_prop = text_hl_selector_get_prop,
                                 .on_destroy = text_hl_selector_on_destroy,
                                 .on_event = text_hl_selector_on_event};

widget_t* text_hl_selector_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h) {
  widget_t* widget = widget_create(parent, TK_REF_VTABLE(text_hl_selector), x, y, w, h);
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL, NULL);

  text_hl_selector->visible_nr = 5;
  text_hl_selector->pressed = FALSE;

  return widget;
}

ret_t text_hl_selector_reset_options(widget_t* widget) {
  text_hl_selector_option_t* iter = NULL;
  text_hl_selector_option_t* next = NULL;
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL, RET_BAD_PARAMS);

  iter = text_hl_selector->option_items;
  while (iter != NULL) {
    next = iter->next;
    TKMEM_FREE(iter);
    iter = next;
  }
  text_hl_selector->option_items = NULL;
  TKMEM_FREE(text_hl_selector->options);

  return RET_OK;
}

ret_t text_hl_selector_append_option(widget_t* widget, int32_t value, const char* text) {
  int32_t size = 0;
  text_hl_selector_option_t* iter = NULL;
  text_hl_selector_option_t* option = NULL;
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);

  return_value_if_fail(text_hl_selector != NULL && text != NULL, RET_BAD_PARAMS);
  size = sizeof(text_hl_selector_option_t) + (strlen(text) + 1) * sizeof(wchar_t);

  option = (text_hl_selector_option_t*)TKMEM_ALLOC(size);
  return_value_if_fail(option != NULL, RET_OOM);

  memset(option, 0x00, size);

  option->value = value;
  tk_utf8_to_utf16(text, option->text, strlen(text) + 1);

  if (text_hl_selector->option_items != NULL) {
    iter = text_hl_selector->option_items;
    while (iter->next != NULL) iter = iter->next;
    iter->next = option;
  } else {
    text_hl_selector->option_items = option;
  }

  return widget_invalidate(widget, NULL);
}

int32_t text_hl_selector_count_options(widget_t* widget) {
  int32_t nr = 0;
  text_hl_selector_option_t* iter = NULL;
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL, nr);

  iter = text_hl_selector->option_items;
  while (iter != NULL) {
    nr++;
    iter = iter->next;
  }

  return nr;
}

int32_t text_hl_selector_get_option_by_value(widget_t* widget, int32_t value) {
  uint32_t i = 0;
  text_hl_selector_option_t* iter = NULL;
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL, -1);

  iter = text_hl_selector->option_items;
  while (iter != NULL) {
    if (value == iter->value) {
      return i;
    }

    i++;
    iter = iter->next;
  }

  return -1;
}

int32_t text_hl_selector_get_option_by_text(widget_t* widget, const char* text) {
  wstr_t str;
  uint32_t i = 0;
  text_hl_selector_option_t* iter = NULL;
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL, -1);

  wstr_init(&str, 0);
  wstr_set_utf8(&str, text);
  iter = text_hl_selector->option_items;
  while (iter != NULL) {
    if (wcscmp(str.str, iter->text) == 0) {
      wstr_reset(&str);
      return i;
    }

    i++;
    iter = iter->next;
  }
  wstr_reset(&str);

  return -1;
}

text_hl_selector_option_t* text_hl_selector_get_option(widget_t* widget, uint32_t index) {
  uint32_t i = 0;
  text_hl_selector_option_t* iter = NULL;
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL, NULL);

  iter = text_hl_selector->option_items;
  while (iter != NULL) {
    if (i == index) {
      return iter;
    }

    i++;
    iter = iter->next;
  }

  return NULL;
}

ret_t text_hl_selector_set_selected_index(widget_t* widget, uint32_t index) {
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  text_hl_selector_option_t* option = text_hl_selector_get_option(widget, index);
  return_value_if_fail(text_hl_selector != NULL, RET_BAD_PARAMS);
  return_value_if_fail(option != NULL, RET_BAD_PARAMS);

  text_hl_selector_set_selected_index_only(text_hl_selector, index);
  text_hl_selector_sync_yoffset_with_selected_index(text_hl_selector);

  return widget_invalidate(widget, NULL);
}

ret_t text_hl_selector_set_visible_nr(widget_t* widget, uint32_t visible_nr) {
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL, RET_BAD_PARAMS);

  text_hl_selector->visible_nr = visible_nr == 3 ? 3 : 5;
  text_hl_selector_sync_yoffset_with_selected_index(text_hl_selector);

  return widget_invalidate(widget, NULL);
}

int32_t text_hl_selector_get_value(widget_t* widget) {
  text_hl_selector_option_t* option = NULL;
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL, -1);

  option = text_hl_selector_get_option(widget, text_hl_selector->selected_index);

  return option != NULL ? option->value : -1;
}

ret_t text_hl_selector_set_value(widget_t* widget, int32_t value) {
  int32_t index = text_hl_selector_get_option_by_value(widget, value);
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL && index >= 0, RET_BAD_PARAMS);

  return text_hl_selector_set_selected_index(widget, index);
}

const char* text_hl_selector_get_text(widget_t* widget) {
  text_hl_selector_option_t* option = NULL;
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL, NULL);

  option = text_hl_selector_get_option(widget, text_hl_selector->selected_index);

  if (option != NULL) {
    str_from_wstr(&(text_hl_selector->text), option->text);

    return text_hl_selector->text.str;
  }

  return NULL;
}

ret_t text_hl_selector_set_text(widget_t* widget, const char* text) {
  int32_t index = text_hl_selector_get_option_by_text(widget, text);
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL && index >= 0, RET_BAD_PARAMS);

  return text_hl_selector_set_selected_index(widget, index);
}

ret_t text_hl_selector_set_loop_mode(widget_t* widget, bool_t loop) {
  text_hl_selector_t* text_hl_selector = TEXT_HL_SELECTOR(widget);
  return_value_if_fail(text_hl_selector != NULL, RET_BAD_PARAMS);
  text_hl_selector->loop = loop;
  return widget_invalidate(widget, NULL);
}

widget_t* text_hl_selector_cast(widget_t* widget) {
  return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, text_hl_selector), NULL);

  return widget;
}
ret_t text_hl_selector_register(void)
{
    return widget_factory_register(widget_factory(), WIDGET_TYPE_TEXT_HL_SELECTOR, text_hl_selector_create);
}
