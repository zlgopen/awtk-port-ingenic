/*
 * WashingModeActivity.c - is provided for use with Ingenic products.
 * No license to Ingenic property rights is granted, Ingenic assumes no
 * liability, provides no warranty either expressed or implied relating
 * to the usage, or intellectual property right infringement except
 * as provided for by Ingenic Terms and Conditions of Sale.
 *
 * All rights reserved by Ingenic Semiconductor CO., LTD.
 *
 * Creator: yflu <yafei.lu@ingenic.com>
 * Maintainer: yflu <yafei.lu@ingenic.com>
 * Created: 2019/09/17
 * Updated:
 */

#include "awtk.h"
#include "base/timer.h"
#include "base/enums.h"
#include "base/assets_manager.h"
#include "widgets/image.h"
#include "widgets/label.h"
#include "tkc/mem.h"
#include "tkc/utils.h"
#include "tkc/utf8.h"
#include "tkc/time_now.h"
#include "ext_widgets/ext_widgets.h"

#include "beverly.h"

static bool_t is_wash_on = FALSE;
static int wash_time_remaining = 0;
static uint32_t wash_timer_id = TK_INVALID_ID;
static int current_index = 0;
static int step_wash_time = 0;
static int step_rinse_time = 0;
static int step_dewater_time = 0;

static ret_t enable_view_wash_progress(widget_t* win, bool_t show, bool_t available)
{
	widget_t* view_wash_progress = widget_lookup(win, "view_wash_progress", TRUE);
	widget_set_visible(view_wash_progress, show, TRUE);
	widget_set_enable(view_wash_progress, available);

	return RET_OK;
}

static ret_t enable_view_wash_step(widget_t* win, bool_t show, bool_t available)
{
	widget_t* view_wash_step = widget_lookup(win, "view_wash_step", TRUE);
	widget_set_visible(view_wash_step, show, TRUE);
	widget_set_enable(view_wash_step, available);

	return RET_OK;
}

static ret_t enable_label_wash_detail(widget_t* win, bool_t show, bool_t available)
{
	widget_t* label_wash_detail = widget_lookup(win, "label_wash_detail", TRUE);
	widget_set_visible(label_wash_detail, show, TRUE);
	widget_set_enable(label_wash_detail, available);

	return RET_OK;
}

static ret_t update_wash_time(widget_t* win, int time)
{
	char time_left[5];
	widget_t* label_wash_time = widget_lookup(win, "label_wash_time", TRUE);
	if (time == -1) {
		widget_set_text_utf8(label_wash_time, "");
		return RET_OK;
	}
	int hour = time / 60;
	int minute = time % 60;
	sprintf(time_left, "%d:%02d", hour, minute);
	widget_set_text_utf8(label_wash_time, time_left);

	return RET_OK;
}

static ret_t update_label_wash_function_name(widget_t* win, char* name)
{
	widget_t* label_wash_name = widget_lookup(win, "label_wash_name", TRUE);
	widget_set_text_utf8(label_wash_name, name);

	return RET_OK;
}

static ret_t update_label_wash_function_detail(widget_t* win, char* description)
{
	widget_t* label_wash_detail = widget_lookup(win, "label_wash_detail", TRUE);
	widget_set_text_utf8(label_wash_detail, description);

	return RET_OK;
}

static ret_t update_wash_status(widget_t* win)
{
	widget_t* image_wash_switch = widget_lookup(win, "image_wash_switch", TRUE);
	widget_t* label_wash_switch = widget_lookup(win, "label_wash_switch", TRUE);
	const char* text_start = locale_info_tr(locale_info(), "wash_start");
	const char* text_pause = locale_info_tr(locale_info(), "wash_pause");

	if (is_wash_on) {
		image_set_image(image_wash_switch, "pause");
		widget_set_text_utf8(label_wash_switch, text_pause);
	} else {
		image_set_image(image_wash_switch, "start");
		widget_set_text_utf8(label_wash_switch, text_start);
	}

	return RET_OK;
}

static ret_t update_wash_step_time(widget_t *win, int type, int time)
{
	widget_t* parent = NULL;
	char result[4];
	switch (type) {
		case STEP_WASH:
			parent = widget_lookup(win, "btn_step_01", TRUE);
			step_wash_time = time;
			break;
		case STEP_RINSE:
			parent = widget_lookup(win, "btn_step_02", TRUE);
			step_rinse_time = time;
			break;
		case STEP_DEWATER:
			parent = widget_lookup(win, "btn_step_03", TRUE);
			step_dewater_time = time;
			break;
	}
	widget_t* label_step_time = widget_lookup(parent, "label_step_time", TRUE);
	sprintf(result, "%d", time);
	widget_set_text_utf8(label_step_time, result);

	return RET_OK;
}

static ret_t update_label_wash_status(widget_t* win)
{
	widget_t* label_wash_status = widget_lookup(win, "label_wash_status", TRUE);
	if (is_wash_on) {
		const char* text_start = locale_info_tr(locale_info(), "wash_status_run");
		widget_set_text_utf8(label_wash_status, text_start);
	} else {
		if (wash_time_remaining) {
			const char* text_pause = locale_info_tr(locale_info(), "wash_status_pause");
			widget_set_text_utf8(label_wash_status, text_pause);
		} else {
			const char* text_finish = locale_info_tr(locale_info(), "wash_status_finish");
			widget_set_text_utf8(label_wash_status, text_finish);
		}
	}

	return RET_OK;
}

static ret_t update_wash_progress(widget_t* win, int remaining)
{
	int total_time;
	if (tk_str_eq(sWashTime[current_index], "")) {
		total_time = 70 * 60;
	} else {
		total_time = atoi(sWashTime[current_index]) * 60;
	}
	int progress = (total_time - remaining) * 10000 / (total_time);
	widget_t* progress_bar = widget_lookup(win, "progress_bar_wash", TRUE);
	progress_bar_set_value(progress_bar, progress);

	return RET_OK;
}

static ret_t update_wash_step_content(widget_t* win, char* step)
{
	widget_t* label_step_time = widget_lookup(win, "label_step_time", TRUE);
	widget_t* label_step_unit = widget_lookup(win, "label_step_unit", TRUE);
	widget_t* label_step_name = widget_lookup(win, "label_step_name", TRUE);
	if (step) {
		int i = 0;
		char* tmp_text = strdup(step);
		char *result = NULL;
		result = strtok(tmp_text,"|" );
		while (result != NULL) {
			i += 1;
			if (i == 1) {
				if (result) {
					widget_set_text_utf8(label_step_time, result);
					if (tk_str_eq(win->name, "btn_step_01")) {
						step_wash_time = atoi(result);
					} else if (tk_str_eq(win->name, "btn_step_02")) {
						step_rinse_time = atoi(result);
					} else if (tk_str_eq(win->name, "btn_step_03")) {
						step_dewater_time = atoi(result);
					}
				} else {
					widget_set_text_utf8(label_step_time, "");
				}
			} else if (i == 2) {
				if (result) {
					widget_set_text_utf8(label_step_unit, result);
				} else {
					widget_set_text_utf8(label_step_unit, "");
				}
			} else if (i == 3) {
				if (result) {
					widget_set_text_utf8(label_step_name, result);
				} else {
					widget_set_text_utf8(label_step_name, "");
				}
			}
			result = strtok( NULL, "|");
		}

		if (tmp_text)
			free(tmp_text);
	}
	return RET_OK;
}

static ret_t update_wash_step(widget_t* win)
{
	widget_t* btn_step_01 = widget_lookup(win, "btn_step_01", TRUE);
	widget_t* btn_step_02 = widget_lookup(win, "btn_step_02", TRUE);
	widget_t* btn_step_03 = widget_lookup(win, "btn_step_03", TRUE);
	if (tk_str_eq(sFuncStepWash[current_index], "") && tk_str_eq(sFuncStepRinse[current_index], "") &&        tk_str_eq(sFuncStepDewater[current_index], "")) {
		enable_view_wash_step(win, FALSE, FALSE);
	} else {
		update_wash_step_content(btn_step_01, sFuncStepWash[current_index]);
		update_wash_step_content(btn_step_02, sFuncStepRinse[current_index]);
		update_wash_step_content(btn_step_03, sFuncStepDewater[current_index]);
	}

	return RET_OK;
}

static ret_t on_wash_funtion_start(const timer_info_t* info)
{
	wash_time_remaining--;
	if (wash_time_remaining) {
		if (wash_time_remaining % 60 == 0) {
			update_wash_time(WIDGET(info->ctx), wash_time_remaining / 60);
		}

	update_wash_progress(WIDGET(info->ctx), wash_time_remaining);
		return RET_REPEAT;
	}

	is_wash_on = !is_wash_on;
	update_label_wash_status(WIDGET(info->ctx));
	return RET_REMOVE;
}

static ret_t on_back_home_button_clicked(void* ctx, event_t* e)
{
	is_wash_on = FALSE;
	wash_time_remaining = 0;

	if (wash_timer_id != TK_INVALID_ID){
		timer_remove(wash_timer_id);
		wash_timer_id = TK_INVALID_ID;
	}
	window_close(WIDGET(ctx));

	widget_use_style(window_manager(), "main_win");
	widget_update_style(window_manager());

	return RET_OK;
}

static ret_t on_wash_function_button_clicked(void* ctx, event_t* e)
{
	start_washing_function_menu_activity();

	return RET_OK;
}

static ret_t on_wash_step_button_clicked(void* ctx, event_t* e)
{
	int type = (int)ctx;
	int time;
	switch (type) {
		case STEP_WASH:
			time = step_wash_time;
			break;
		case STEP_RINSE:
			time = step_rinse_time;
			break;
		case STEP_DEWATER:
			time = step_dewater_time;
			break;
	}

	start_washing_step_activity(type, time);

	return RET_OK;
}

static ret_t on_wash_step_value_changed(void* ctx, event_t* e)
{
	WashingStepValue* step_value = (WashingStepValue*)((prop_change_event_t*)e)->value;
	update_wash_step_time(WIDGET(ctx), step_value->type, step_value->result);

	return RET_OK;
}

static ret_t on_wash_switch_button_clicked(void* ctx, event_t* e)
{
	if (e->type == EVT_LONG_PRESS) {
		if (is_wash_on) {
			is_wash_on = !is_wash_on;
			wash_time_remaining = 0;

			update_wash_status(WIDGET(ctx));
			if (tk_str_eq(sWashTime[current_index], "")) {
				update_wash_time(WIDGET(ctx), -1);
			} else {
				update_wash_time(WIDGET(ctx), atoi(sWashTime[current_index]));
			}
			enable_view_wash_progress(WIDGET(ctx), is_wash_on, is_wash_on);
			if (tk_str_eq(sFuncStepWash[current_index], "") && tk_str_eq(sFuncStepRinse[current_index], "") &&   tk_str_eq(sFuncStepDewater[current_index], "")) {
				enable_view_wash_step(WIDGET(ctx), FALSE, FALSE);
			} else {
				enable_view_wash_step(WIDGET(ctx), !is_wash_on, !is_wash_on);
			}
			enable_label_wash_detail(WIDGET(ctx), !is_wash_on, !is_wash_on);

			if (wash_timer_id != TK_INVALID_ID){
				timer_remove(wash_timer_id);
				wash_timer_id = TK_INVALID_ID;
			}
		}
	} else if (e->type == EVT_CLICK) {
		is_wash_on = !is_wash_on;
		if (is_wash_on) {
			if (wash_time_remaining == 0) {
				enable_view_wash_progress(WIDGET(ctx), is_wash_on, is_wash_on);
				enable_view_wash_step(WIDGET(ctx), !is_wash_on, !is_wash_on);
				enable_label_wash_detail(WIDGET(ctx), !is_wash_on, !is_wash_on);
				if (tk_str_eq(sWashTime[current_index], "")) {
					wash_time_remaining = 70 * 60;
					update_wash_time(WIDGET(ctx), wash_time_remaining / 60);
				} else {
					wash_time_remaining = atoi(sWashTime[current_index]) * 60;
				}
				update_wash_progress(WIDGET(ctx), wash_time_remaining);
			}
			wash_timer_id = timer_add(on_wash_funtion_start, WIDGET(ctx), 1000);
		} else {
			if (wash_timer_id != TK_INVALID_ID){
				timer_remove(wash_timer_id);
				wash_timer_id = TK_INVALID_ID;
			}
		}

		update_wash_status(WIDGET(ctx));
		update_label_wash_status(WIDGET(ctx));
	}

	return RET_OK;
}

static ret_t init_washing_mode_activity_widgets_events(void* ctx, const void* iter)
{
	widget_t* widget = WIDGET(iter);
	widget_t* win = widget_get_window(widget);
	if (widget->name != NULL) {
		const char* name = widget->name;
		if (tk_str_eq(name, "btn_wash_function")) {
			widget_on(widget, EVT_CLICK, on_wash_function_button_clicked, win);
		} else if (tk_str_eq(name, "btn_wash_switch")) {
			widget_on(widget, EVT_CLICK, on_wash_switch_button_clicked, win);
			widget_on(widget, EVT_LONG_PRESS, on_wash_switch_button_clicked, win);
		} else if (tk_str_eq(name, "btn_step_01")) {
			widget_on(widget, EVT_CLICK, on_wash_step_button_clicked, (void*)STEP_WASH);
		} else if (tk_str_eq(name, "btn_step_02")) {
			widget_on(widget, EVT_CLICK, on_wash_step_button_clicked, (void*)STEP_RINSE);
		} else if (tk_str_eq(name, "btn_step_03")) {
			widget_on(widget, EVT_CLICK, on_wash_step_button_clicked, (void*)STEP_DEWATER);
		} else if (tk_str_eq(name, "label_step_time") || tk_str_eq(name, "label_step_unit") || tk_str_eq(name, "label_step_name")) {
			widget_set_enable(widget, FALSE);
		}
	}

	return RET_OK;
}

static ret_t init_washing_mode_activity(widget_t* win, int index)
{
	is_wash_on = FALSE;
	current_index = index;
	wash_time_remaining = 0;

	if (tk_str_eq(sWashTime[index], "")) {
		update_wash_time(win, -1);
	} else {
		update_wash_time(win, atoi(sWashTime[index]));
	}

	update_wash_step(win);
	update_label_wash_function_name(win, sFunction[index]);
	update_label_wash_function_detail(win, sFuncDescription[index]);

	return RET_OK;
}

ret_t start_washing_mode_activity(int index)
{
	widget_use_style(window_manager(), "washing_win");
	widget_t* mode_win = window_open("washing_mode_activity");
	init_washing_mode_activity(mode_win, index);
	widget_foreach(mode_win, init_washing_mode_activity_widgets_events, mode_win);
	widget_on(mode_win, EVT_BACK_TO_HOME, on_back_home_button_clicked, mode_win);
	widget_on(mode_win, EVT_WASH_STEP_TIME_CHANGED, on_wash_step_value_changed, mode_win);

	return RET_OK;
}
