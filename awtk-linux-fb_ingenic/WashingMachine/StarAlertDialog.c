/*
 * StarAlertDialog.c - is provided for use with Ingenic products.
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
 * Updated: 2019/09/23
 */

#include "awtk.h"
#include "beverly.h"

static ret_t update_label_favorite_alert(widget_t* win, char* content)
{
	widget_t* label_favorite_alert = widget_lookup(win, "label_favorite_alert", TRUE);
	log_debug("%s: content: %s\n", __func__, content);
	widget_set_text_utf8(label_favorite_alert, content);

	return RET_OK;
}

static ret_t init_star_alert_dialog(widget_t* win, bool_t favorite)
{
	const char* text_favorite_add = locale_info_tr(locale_info(), "favorite_add");
	const char* text_favorite_cancel = locale_info_tr(locale_info(), "favorite_cancel");

	if (favorite) {
		update_label_favorite_alert(win, text_favorite_add);
	} else {
		update_label_favorite_alert(win, text_favorite_cancel);
	}

	return RET_OK;
}

ret_t show_star_alert_dialog(bool_t favorite)
{
	widget_t* alert_dialog = window_open("star_alert_dialog");
	init_star_alert_dialog(alert_dialog, favorite);

	return RET_OK;
}

