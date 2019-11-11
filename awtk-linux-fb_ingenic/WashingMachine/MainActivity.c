/*
 * MainActivity.c - is provided for use with Ingenic products.
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
#include "beverly.h"

static bool_t is_star = FALSE;

static ret_t enable_button_favorite(widget_t* win, bool_t show, bool_t available)
{
	widget_t* btn_favorite = widget_lookup(win, "btn_favorite", TRUE);
	widget_set_visible(btn_favorite, show, TRUE);
	widget_set_enable(btn_favorite, available);

	return RET_OK;
}

static ret_t update_favorite_button_status(widget_t* win, bool_t favorite)
{
	widget_t* check_btn_favorite = widget_lookup(win, "btn_favorite", TRUE);
	check_button_set_value(check_btn_favorite, favorite);

	return RET_OK;
}

static ret_t on_favorite_button_clicked(void* ctx, event_t* e)
{
	is_star = !is_star;
	show_star_alert_dialog(is_star);

	return RET_OK;
}

static ret_t on_home_button_clicked(void* ctx, event_t* e)
{
	if (is_star) {
		is_star = !is_star;
		update_favorite_button_status(WIDGET(ctx), FALSE);
	}

	enable_button_favorite(WIDGET(ctx), FALSE, FALSE);
	widget_t* top_win = window_manager_get_top_main_window(window_manager());
	if (top_win && top_win->name != NULL) {
		const char* name = top_win->name;
		if (tk_str_eq(name, "washing_mode_activity")) {
			log_debug("leave washing mode activity\n");
			event_t evt = event_init(EVT_BACK_TO_HOME, top_win);
			widget_dispatch(top_win, &evt);
			widget_use_style(window_manager(), "main_win");
//			widget_invalidate(window_manager(), NULL);
		}
	}

	return RET_OK;
}

static ret_t on_setting_button_clicked(void* ctx, event_t* e)
{
	start_settings_activity();

	return RET_OK;
}
static ret_t on_scroll_view_up_before(void* ctx, event_t* e)
{
    scroll_view_t* scroll_view = SCROLL_VIEW(ctx);
    ret_t ret = RET_OK;
    if(scroll_view->dragged)
        ret = RET_STOP;
    e->type = EVT_POINTER_UP;
    widget_dispatch(WIDGET(ctx), e);
    return ret;
}
static ret_t on_wash_function_item_clicked(void* ctx, event_t* e)
{
	int index = (int)ctx;
	widget_t* system_bar = widget_lookup(window_manager(), "system_bar", TRUE);
	enable_button_favorite(system_bar, TRUE, TRUE);
	start_washing_mode_activity(index);

	return RET_OK;
}

static ret_t init_main_activity_widgets_events(void* ctx, const void* iter)
{
	widget_t* widget = WIDGET(iter);
	widget_t* win = widget_get_window(widget);
	if (widget->name != NULL) {
		const char* name = widget->name;
		if (tk_str_eq(name, "btn_favorite")) {
			widget_on(widget, EVT_CLICK, on_favorite_button_clicked, win);
		} else if (tk_str_eq(name, "btn_home")) {
			widget_on(widget, EVT_CLICK, on_home_button_clicked, win);
		} else if (tk_str_eq(name, "btn_setting")) {
			widget_on(widget, EVT_CLICK, on_setting_button_clicked, win);
		} else if (tk_str_eq(name, "scroll_view")) {
			int i = 0;
//            widget_on(widget,EVT_POINTER_UP_BEFORE_CHILDREN,on_scroll_view_up_before,widget);
			for (i = 0; i < 18; i++) {
				widget_t* list_item = widget_get_child(widget, i);
				widget_on(list_item, EVT_CLICK, on_wash_function_item_clicked, (void*)i);
			}
		}
	}

	return RET_OK;
}

ret_t application_init()
{
	tk_ext_widgets_init();
	text_hl_selector_register();
	widget_use_style(window_manager(), "main_win");
	widget_t* system_bar = window_open("system_bar");
	widget_t* main_win = window_open("main_activity");
	enable_button_favorite(system_bar, FALSE, FALSE);
	widget_foreach(system_bar, init_main_activity_widgets_events, system_bar);
	widget_foreach(main_win, init_main_activity_widgets_events, main_win);

	return RET_OK;
}

static ret_t assets_init(void)
{
	assets_manager_t* rm = assets_manager();
	assets_manager_load(rm, ASSET_TYPE_STYLE, "default");
	assets_manager_load(rm, ASSET_TYPE_FONT, "default");
	tk_init_assets();

	return RET_OK;
}

int main(void)
{
	tk_init(1280, 400, APP_SIMULATOR, NULL, "/release");
	tk_set_lcd_orientation(LCD_ORIENTATION_90);
	assets_init();
	application_init();
	locale_info_change(locale_info(), "zh", "CN");

#ifdef ENABLE_CURSOR
	window_manager_set_cursor(window_manager(), "cursor");
#endif /*ENABLE_CURSOR*/

	tk_run();

	return 0;
}

