/*
 * WashingStepActivity.c - is provided for use with Ingenic products.
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

static int step_type;

static ret_t on_wash_step_time_changed(void* ctx, event_t* e)
{
	widget_t* win = WIDGET(ctx);
	widget_t* step_selector = widget_lookup(win, "step_selector", TRUE);
	widget_t* parent = widget_lookup(window_manager(), "washing_mode_activity", TRUE);
	int result = widget_get_value(step_selector);
	WashingStepValue value;
	value.type = step_type;
	value.result = result;
	prop_change_event_t evt;
    evt.e = event_init(EVT_WASH_STEP_TIME_CHANGED, win);
    evt.value = &value;
    widget_dispatch(parent, (event_t*)&evt);

	return RET_OK;
}


static ret_t init_washing_step_activity_widgets_events(void* ctx, const void* iter)
{
	widget_t* widget = WIDGET(iter);
	widget_t* win = widget_get_window(widget);
	if (widget->name != NULL) {
		const char* name = widget->name;
		if (tk_str_eq(name, "step_selector")) {
			widget_on(widget, EVT_VALUE_CHANGED, on_wash_step_time_changed, win);
		}
	}

	return RET_OK;
}

static ret_t init_washing_step_activity(widget_t* win, int step, int result)
{
	widget_t* step_selector = widget_lookup(win, "step_selector", TRUE);
	switch (step) {
		case STEP_WASH:
			text_hl_selector_set_range_options(step_selector, 5, 36, 1);
			break;
		case STEP_RINSE:
			text_hl_selector_set_range_options(step_selector, 1, 5, 1);
			break;
		case STEP_DEWATER:
			text_hl_selector_set_range_options(step_selector, 5, 26, 1);
			break;
	}
	text_hl_selector_set_value(step_selector, result);

	return RET_OK;
}

ret_t start_washing_step_activity(int step, int result)
{
	widget_t* step_win = window_open("washing_step_activity");
	step_type = step;
	init_washing_step_activity(step_win, step, result);
    widget_foreach(step_win, init_washing_step_activity_widgets_events, step_win);

	return RET_OK;
}

