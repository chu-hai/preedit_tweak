/*-------------------------------------------------------------------

 gtk_compat.h

 Copyright (c) 2017 Chuhai

 This file is part of Preedit Tweak, a Geany plugin.
 This software is released under the MIT License, see LICENSE.txt.

-------------------------------------------------------------------*/

#ifndef _GTK_COMPAT_H_
#define _GTK_COMPAT_H_

#include <geanyplugin.h>

#if (GTK_MAJOR_VERSION == 2)
	/* Gtk 2 */
	#define	USE_GTK2

	#define	GDK_COLOR	GdkColor

	#define	GDK_COLOR_PARSE(SPEC, COLOR) \
			gdk_color_parse(SPEC, COLOR)
	#define	GTK_COLOR_BUTTON_GET_COLOR(BUTTON, COLOR) \
			gtk_color_button_get_color(GTK_COLOR_BUTTON(BUTTON), COLOR)
	#define	GTK_COLOR_BUTTON_SET_COLOR(BUTTON, COLOR) \
			gtk_color_button_set_color(GTK_COLOR_BUTTON(BUTTON), COLOR)
#elif (GTK_MAJOR_VERSION == 3)
	/* Gtk 3 */
	#define	USE_GTK3

	#define	GDK_COLOR	GdkRGBA

	#define	GDK_COLOR_PARSE(SPEC, COLOR) \
			gdk_rgba_parse(COLOR, SPEC)
	#define	GTK_COLOR_BUTTON_GET_COLOR(BUTTON, COLOR) \
			gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(BUTTON), COLOR)
	#define	GTK_COLOR_BUTTON_SET_COLOR(BUTTON, COLOR) \
			gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(BUTTON), COLOR)
#else
	#error "Unsupported Gtk version. "
#endif

#endif /* _GTK_COMPAT_H_ */
