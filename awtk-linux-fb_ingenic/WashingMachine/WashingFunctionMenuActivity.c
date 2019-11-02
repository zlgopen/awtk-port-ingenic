/*
 * WashingFunctionMenuActivity.c - is provided for use with Ingenic products.
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
 * Updated: 2019/09/20
 */

#include "awtk.h"
#include "beverly.h"

static bool_t foggy_wash_switch = FALSE;
static bool_t timed_wash_switch = FALSE;

static ret_t update_foggy_wash_switch_status(widget_t* win)
{
	widget_t* image_foggy_wash = widget_lookup(win, "image_foggy_wash", TRUE);
	if (foggy_wash_switch) {
		image_set_image(image_foggy_wash, "fog_wash_sel");
	} else {
		image_set_image(image_foggy_wash, "fog_wash");
	}

	return RET_OK;
}

static ret_t update_timed_wash_switch_status(widget_t* win)
{
	widget_t* image_timed_wash = widget_lookup(win, "image_timed_wash", TRUE);
	if (timed_wash_switch) {
		image_set_image(image_timed_wash, "timed_wash_sel");
	} else {
		image_set_image(image_timed_wash, "timed_wash");
	}

	return RET_OK;
}

static ret_t on_foggy_wash_button_clicked(void* ctx, event_t* e)
{
	foggy_wash_switch = !foggy_wash_switch;
	update_foggy_wash_switch_status(WIDGET(ctx));

	return RET_OK;
}

static ret_t on_timed_wash_button_clicked(void* ctx, event_t* e)
{
	timed_wash_switch = !timed_wash_switch;
	update_timed_wash_switch_status(WIDGET(ctx));

	return RET_OK;
}

static ret_t init_washing_function_menu_activity_widgets_events(void* ctx, const void* iter)
{
	widget_t* widget = WIDGET(iter);
	widget_t* win = widget_get_window(widget);
	if (widget->name != NULL) {
		const char* name = widget->name;
		if (tk_str_eq(name, "btn_foggy_wash")) {
			widget_on(widget, EVT_CLICK, on_foggy_wash_button_clicked, win);
		} else if (tk_str_eq(name, "btn_timed_wash")) {
			widget_on(widget, EVT_CLICK, on_timed_wash_button_clicked, win);
		}
	}

	return RET_OK;
}

static ret_t init_washing_function_menu_activity(widget_t* win)
{
	foggy_wash_switch = FALSE;
	timed_wash_switch = FALSE;

	return RET_OK;
}

ret_t start_washing_function_menu_activity()
{
	widget_t* func_win = window_open("washing_function_menu_activity");
	init_washing_function_menu_activity(func_win);
	widget_foreach(func_win, init_washing_function_menu_activity_widgets_events, func_win);

	return RET_OK;
}
