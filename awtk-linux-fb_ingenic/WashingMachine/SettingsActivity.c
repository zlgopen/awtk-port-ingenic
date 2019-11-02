/*
 * SettingsActivity.c - is provided for use with Ingenic products.
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

static bool_t mute_switch = FALSE;
static bool_t childlock_switch = FALSE;
static bool_t light_switch = FALSE;

static ret_t update_button_childlock_status(widget_t* win, bool_t status)
{
	widget_t* image_childlock = widget_lookup(win, "image_childlock", TRUE);
	if (status) {
		image_set_image(image_childlock, "childlock_select");
	} else {
		image_set_image(image_childlock, "childlock");
	}

	return RET_OK;
}

static ret_t update_button_light_status(widget_t* win, bool_t status)
{
	widget_t* image_light = widget_lookup(win, "image_light", TRUE);
	if (status) {
		image_set_image(image_light, "light_select");
	} else {
		image_set_image(image_light, "light");
	}

	return RET_OK;
}

static ret_t update_button_mute_status(widget_t* win, bool_t status)
{
	widget_t* image_mute = widget_lookup(win, "image_mute", TRUE);
	if (status) {
		image_set_image(image_mute, "mute_select");
	} else {
		image_set_image(image_mute, "mute");
	}

	return RET_OK;
}

static ret_t on_childlock_button_clicked(void* ctx, event_t* e)
{
	childlock_switch = !childlock_switch;
	update_button_childlock_status(WIDGET(ctx), childlock_switch);

	return RET_OK;
}

static ret_t on_light_button_clicked(void* ctx, event_t* e)
{
	light_switch = !light_switch;
	update_button_light_status(WIDGET(ctx), light_switch);

	return RET_OK;
}

static ret_t on_mute_button_clicked(void* ctx, event_t* e)
{
	mute_switch = !mute_switch;
	update_button_mute_status(WIDGET(ctx), mute_switch);

	return RET_OK;
}

static ret_t init_settings_activity_widgets_events(void* ctx, const void* iter)
{
	widget_t* widget = WIDGET(iter);
	widget_t* win = widget_get_window(widget);
	if (widget->name != NULL) {
		const char* name = widget->name;
		if (tk_str_eq(name, "btn_childlock")) {
			widget_on(widget, EVT_CLICK, on_childlock_button_clicked, win);
		} else if (tk_str_eq(name, "btn_light")) {
			widget_on(widget, EVT_CLICK, on_light_button_clicked, win);
		} else if (tk_str_eq(name, "btn_mute")) {
			widget_on(widget, EVT_CLICK, on_mute_button_clicked, win);
		}
	}

	return RET_OK;
}

static ret_t init_settings_activity(widget_t* win)
{
	update_button_childlock_status(win, childlock_switch);
	update_button_light_status(win, light_switch);
	update_button_mute_status(win, mute_switch);

	return RET_OK;
}

ret_t start_settings_activity()
{
	widget_t* setting_win = window_open("settings_activity");
	init_settings_activity(setting_win);
	widget_foreach(setting_win, init_settings_activity_widgets_events, setting_win);

	return RET_OK;
}
