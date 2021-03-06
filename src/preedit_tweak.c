/*-------------------------------------------------------------------

 preedit_tweak.c

 Copyright (c) 2017 Chuhai

 This file is part of Preedit Tweak, a Geany plugin.
 This software is released under the MIT License, see LICENSE.txt.

-------------------------------------------------------------------*/

#include "config.h"
#include <geanyplugin.h>
#include <math.h>

#include "gtk_compat.h"

#define	INDICATOR_COUNT				2

#define	MSG_KEYFILE_SAVE_FAILED		N_("Failed to save configuration file.\n\nFilename: '%s'\n\n%s")
#define MSG_DIR_CREATE_FAILED		N_("Failed to create directory.\n\nDirectory: '%s'\n\n%s")


/*--------------------------------------------------*/
/*	IME Indicator Definition						*/
/*--------------------------------------------------*/
typedef struct {
	gchar		*name;
	gchar		*desc;
	int			indicator;
	int			indicator_style;
	guint32		fore_color;
} IndicatorDefinition;

static const IndicatorDefinition indicator_defs[INDICATOR_COUNT] = {
	{"Input",	N_("Input String"),		INDIC_IME + 0,	INDIC_DOTS,			0x0000ff},
	{"Target",	N_("Target String"),	INDIC_IME + 1,	INDIC_STRAIGHTBOX,	0x0000ff}
};


/*--------------------------------------------------*/
/*	Combobox Item Definition						*/
/*--------------------------------------------------*/
typedef struct {
	gchar	*desc;
	int		indicator_style;
} AttrComboBoxItem;

static const AttrComboBoxItem combobox_item[] = {
	{N_("No visual effect"),				INDIC_HIDDEN},
	{N_("Foreground color"),				INDIC_TEXTFORE},
	{N_("Underline (Squiggle 1)"),			INDIC_SQUIGGLE},
	{N_("Underline (Squiggle 2)"),			INDIC_SQUIGGLELOW},
	{N_("Underline (Squiggle 3)"),			INDIC_SQUIGGLEPIXMAP},
	{N_("Underline (T Shapes)"),			INDIC_TT},
	{N_("Underline (Diagonal hatching)"),	INDIC_DIAGONAL},
	{N_("Underline (Dash)"),				INDIC_DASH},
	{N_("Underline (Dot)"),					INDIC_DOTS},
	{N_("Underline (Thick-line)"),			INDIC_COMPOSITIONTHICK},
	{N_("Underline (Thin-line)"),			INDIC_COMPOSITIONTHIN},
	{N_("Strikethrough"),					INDIC_STRIKE},
	{N_("Box (Normal border)"),				INDIC_BOX},
	{N_("Box (Dot border)"),				INDIC_DOTBOX},
	{N_("Box and Fill (Rounded border)"),	INDIC_ROUNDBOX},
	{N_("Box and Fill (Normal border 1)"),	INDIC_STRAIGHTBOX},
	{N_("Box and Fill (Normal border 2)"),	INDIC_FULLBOX},
	{NULL, 0}
};


/*--------------------------------------------------*/
/*	Configuration Dialog Data Declaration			*/
/*--------------------------------------------------*/
typedef struct {
	GtkWidget	*hbox;
	GtkWidget	*label_attr;
	GtkWidget	*combo_attr;
	GtkWidget	*color_attr;
} UI_AttrItem;


typedef struct {
	GtkWidget	*vbox_base;
	GtkWidget	*chk_enable_inline;
	GtkWidget	*frame_appearance;
	GtkWidget	*vbox_appearance;
	UI_AttrItem	attr[INDICATOR_COUNT];
} UI_ConfigDialog;


/*--------------------------------------------------*/
/*	Plugin Data Definition							*/
/*--------------------------------------------------*/
typedef struct {
	gchar		*name;
	int			indicator;
	int			indicator_style;
	GDK_COLOR	fore_color;
} AttrDataStore;

typedef struct {
	GString			*plugin_name;
	GString			*config_file;
	GeanyData		*geany_data;
	gboolean		enable_inline_preedit;
	AttrDataStore	initial_attr[INDICATOR_COUNT];
	AttrDataStore	current_attr[INDICATOR_COUNT];
} PreeditTweakData;

static PreeditTweakData *pt_data = NULL;


/*--------------------------------------------------*/
/*	PluginCallback Definition						*/
/*--------------------------------------------------*/
static void on_new_document(GObject *obj, GeanyDocument *doc, gpointer user_data);
static PluginCallback plugin_callbacks[] = {
	{"document-new",  (GCallback) &on_new_document, TRUE, NULL},
	{"document-open", (GCallback) &on_new_document, TRUE, NULL},
	{NULL, NULL, FALSE, NULL}
};


/*--------------------------------------------------*/
/*	Function Definition								*/
/*--------------------------------------------------*/
static GDK_COLOR int_to_color(guint32 value)
{
	GDK_COLOR color;
#ifdef USE_GTK2
	color.red    = ((value & 0xff0000) >> 16) * 0x101;
	color.green  = ((value & 0x00ff00) >>  8) * 0x101;
	color.blue   = ((value & 0x0000ff) >>  0) * 0x101;
#else
	color.red    = (double)((value & 0xff0000) >> 16) / 255;
	color.green  = (double)((value & 0x00ff00) >>  8) / 255;
	color.blue   = (double)((value & 0x0000ff) >>  0) / 255;
	color.alpha  = 1.0;
#endif
	return color;
}

static guint32 color_to_int(const GDK_COLOR *color)
{
#ifdef USE_GTK2
	return (((color->red   / 0x101) << 16) |
			((color->green / 0x101) <<  8) |
			((color->blue  / 0x101) <<  0));
#else
	return (((guint32)floor(color->red   * 255 + 0.5) << 16) |
			((guint32)floor(color->green * 255 + 0.5) <<  8) |
			((guint32)floor(color->blue  * 255 + 0.5) <<  0));
#endif
}

static guint32 color_to_scicolor(GDK_COLOR *color)
{
#ifdef USE_GTK2
	return (((color->red   / 0x101) <<  0) |
			((color->green / 0x101) <<  8) |
			((color->blue  / 0x101) << 16));
#else
	return (((guint32)floor(color->red   * 255 + 0.5) <<  0) |
			((guint32)floor(color->green * 255 + 0.5) <<  8) |
			((guint32)floor(color->blue  * 255 + 0.5) << 16));
#endif
}

static void change_preedit_mode(GeanyDocument *doc)
{
	int ime_interaction = (pt_data->enable_inline_preedit) ? SC_IME_INLINE : SC_IME_WINDOWED;
	ScintillaObject	*sci = doc->editor->sci;

	scintilla_send_message(sci, SCI_SETIMEINTERACTION, ime_interaction, 0);
}

static void change_preedit_attr(GeanyDocument *doc, AttrDataStore attr[])
{
	guint			i;
	ScintillaObject	*sci = doc->editor->sci;

	for (i = 0; i < INDICATOR_COUNT; i++) {
		scintilla_send_message(sci, SCI_INDICSETSTYLE, attr[i].indicator, attr[i].indicator_style);
		scintilla_send_message(sci, SCI_INDICSETFORE,  attr[i].indicator, color_to_scicolor(&(attr[i].fore_color)));
	}
}

static guint get_combobox_index(const int indicator_style)
{
	guint	i;

	for (i = 0; combobox_item[i].desc; i++) {
		if (combobox_item[i].indicator_style == indicator_style) {
			return i;
		}
	}

	return -1;
}


/*--------------------------------------------------*/
/*	Callback Function Definition					*/
/*--------------------------------------------------*/
static void on_new_document(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
	change_preedit_mode(doc);
	change_preedit_attr(doc, pt_data->current_attr);
}

static gboolean on_init_preedit_tweak(GeanyPlugin *plugin, gpointer pdata)
{
	GeanyData		*geany_data = plugin->geany_data;
	guint		 	i;
	AttrDataStore	*attr;
	GKeyFile		*config;
	gchar			*color_work;

	/* initialize */
	pt_data = g_new0(PreeditTweakData, 1);
	pt_data->plugin_name = g_string_new(PACKAGE);
	pt_data->config_file = g_string_new(g_build_path(G_DIR_SEPARATOR_S,
													 geany_data->app->configdir,
													 "plugins", pt_data->plugin_name->str,
													 "preedit_tweak.conf", NULL));
	pt_data->geany_data = geany_data;
	pt_data->enable_inline_preedit = FALSE;

	for (i = 0; i < INDICATOR_COUNT; i++) {
		attr = &pt_data->initial_attr[i];
		attr->name				= indicator_defs[i].name;
		attr->indicator			= indicator_defs[i].indicator;
		attr->indicator_style	= indicator_defs[i].indicator_style;
		attr->fore_color		= int_to_color(indicator_defs[i].fore_color);

		pt_data->current_attr[i] = *attr;
	}

	/* load settings from config file */
	config = g_key_file_new();
	if (g_key_file_load_from_file(config, pt_data->config_file->str, G_KEY_FILE_KEEP_COMMENTS, NULL)) {
		pt_data->enable_inline_preedit = g_key_file_get_boolean(config, "general", "enable_inline_preedit", NULL);
		for (i = 0; i < INDICATOR_COUNT; i++) {
			attr = &pt_data->current_attr[i];
			attr->indicator_style = g_key_file_get_integer(config, "indicator_style", attr->name, NULL);
			color_work = g_key_file_get_string(config, "fore_color", attr->name, NULL);
			GDK_COLOR_PARSE(color_work, &attr->fore_color);
			g_free(color_work);
		}
	}
	g_key_file_free(config);

	/* apply settings */
	foreach_document(i) {
		change_preedit_mode(documents[i]);
		change_preedit_attr(documents[i], pt_data->current_attr);
	}

	return TRUE;
}

static void on_cleanup_preedit_tweak(GeanyPlugin *plugin, gpointer pdata)
{
	GeanyData	*geany_data = pt_data->geany_data;
	guint		i;

	/* forced to disable the inline preedit and reset appearance */
	pt_data->enable_inline_preedit = FALSE;
	foreach_document(i) {
		change_preedit_mode(documents[i]);
		change_preedit_attr(documents[i], pt_data->initial_attr);
	}

	/* free allocated memory */
	g_string_free(pt_data->plugin_name, TRUE);
	g_string_free(pt_data->config_file, TRUE);
	g_free(pt_data);
}

static void on_configure_response(GtkDialog *dialog, gint response, UI_ConfigDialog *ui)
{
	GeanyData		*geany_data = pt_data->geany_data;
	guint		 	i;
	guint			idx;
	AttrDataStore	*attr;
	UI_AttrItem		*ui_attr;
	GKeyFile		*config;
	gchar			*config_dir, *write_data;
	gint			ret;
	GString			*color_work;

	if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_APPLY) {
		/* get plugin data from dialog */
		pt_data->enable_inline_preedit = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ui->chk_enable_inline));

		for (i = 0; i < INDICATOR_COUNT; i++) {
			attr = &pt_data->current_attr[i];
			ui_attr = &ui->attr[i];

			GTK_COLOR_BUTTON_GET_COLOR(GTK_COLOR_BUTTON(ui_attr->color_attr), &attr->fore_color);
			idx = gtk_combo_box_get_active(GTK_COMBO_BOX(ui_attr->combo_attr));
			attr->indicator_style = combobox_item[idx].indicator_style;
		}

		/* save settings to config file */
		config = g_key_file_new();
		color_work = g_string_new("");
		g_key_file_load_from_file(config, pt_data->config_file->str, G_KEY_FILE_KEEP_COMMENTS, NULL);
		g_key_file_set_boolean(config, "general", "enable_inline_preedit", pt_data->enable_inline_preedit);
		for (i = 0; i < INDICATOR_COUNT; i++) {
			attr = &pt_data->current_attr[i];
			g_key_file_set_integer(config, "indicator_style", attr->name, attr->indicator_style);
			g_string_sprintf(color_work, "#%.6x", color_to_int(&attr->fore_color));
			g_key_file_set_string(config, "fore_color", attr->name, color_work->str);
		}

		config_dir = g_path_get_dirname(pt_data->config_file->str);
		ret = utils_mkdir(config_dir, TRUE);
		if (ret == 0) {
			write_data = g_key_file_to_data(config, NULL, NULL);
			ret = utils_write_file(pt_data->config_file->str, write_data);
			if (ret != 0) {
				dialogs_show_msgbox(GTK_MESSAGE_ERROR, _(MSG_KEYFILE_SAVE_FAILED), pt_data->config_file->str, g_strerror(ret));
			}
			g_free(write_data);
		}
		else {
			dialogs_show_msgbox(GTK_MESSAGE_ERROR, _(MSG_DIR_CREATE_FAILED), config_dir, g_strerror(ret));
		}

		g_free(config_dir);
		g_string_free(color_work, TRUE);
		g_key_file_free(config);

		/* apply settings */
		foreach_document(i) {
			change_preedit_mode(documents[i]);
			change_preedit_attr(documents[i], pt_data->current_attr);
		}
	}
}

static GtkWidget *on_configure_preedit_tweak(GeanyPlugin *plugin, GtkDialog *dialog, gpointer data)
{
	guint			i, j;
	guint			idx;
	AttrDataStore	*attr;
	UI_AttrItem		*ui_attr;
	UI_ConfigDialog	*ui = g_new0(UI_ConfigDialog, 1);

	/* base vbox */
	ui->vbox_base = gtk_vbox_new(FALSE, 0);

	/* check button */
	ui->chk_enable_inline = gtk_check_button_new_with_label(_("Enable inline preediting"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->chk_enable_inline), pt_data->enable_inline_preedit);
	gtk_box_pack_start(GTK_BOX(ui->vbox_base), ui->chk_enable_inline, FALSE, TRUE, 10);

	/* appearance frame */
	ui->frame_appearance = gtk_frame_new(_("Appearance of inline preedit"));
	gtk_box_pack_start(GTK_BOX(ui->vbox_base), ui->frame_appearance, TRUE, TRUE, 10);
	ui->vbox_appearance = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(ui->frame_appearance), ui->vbox_appearance);

	/* indicator attributes */
	for (i = 0; i < INDICATOR_COUNT; i++) {
		attr = &pt_data->current_attr[i];
		ui_attr = &ui->attr[i];
		ui_attr->hbox = gtk_hbox_new(FALSE, 10);
		ui_attr->label_attr = gtk_label_new(_(indicator_defs[i].desc));
		ui_attr->combo_attr = gtk_combo_box_text_new();
		ui_attr->color_attr = gtk_color_button_new();

		for (j = 0; combobox_item[j].desc; j++) {
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ui_attr->combo_attr), _(combobox_item[j].desc));
		}

		idx = get_combobox_index(attr->indicator_style);
		if (idx != -1) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(ui_attr->combo_attr), idx);
		}
		GTK_COLOR_BUTTON_SET_COLOR(GTK_COLOR_BUTTON(ui_attr->color_attr), &(attr->fore_color));

		gtk_box_pack_start(GTK_BOX(ui->vbox_appearance), ui_attr->hbox, FALSE, FALSE, 6);
		gtk_box_pack_start(GTK_BOX(ui_attr->hbox), ui_attr->label_attr, FALSE, FALSE, 6);
		gtk_box_pack_end  (GTK_BOX(ui_attr->hbox), ui_attr->color_attr, FALSE, FALSE, 6);
		gtk_box_pack_end  (GTK_BOX(ui_attr->hbox), ui_attr->combo_attr, FALSE, FALSE, 6);
	}

	g_signal_connect_data(dialog, "response", G_CALLBACK(on_configure_response),
						  ui, (GClosureNotify)g_free, 0);

	return ui->vbox_base;
}

G_MODULE_EXPORT void geany_load_module(GeanyPlugin *plugin)
{
#ifdef ENABLE_NLS
	main_locale_init(PACKAGE_LOCALE_DIR, GETTEXT_PACKAGE);
#endif
	plugin->info->name = "Preedit Tweak";
	plugin->info->description = _("Tweaks for the preedit appearances.");
	plugin->info->version = PACKAGE_VERSION;
	plugin->info->author = "Chuhai";
	plugin->funcs->init = on_init_preedit_tweak;
	plugin->funcs->cleanup = on_cleanup_preedit_tweak;
	plugin->funcs->configure = on_configure_preedit_tweak;
	plugin->funcs->callbacks = plugin_callbacks;
	GEANY_PLUGIN_REGISTER(plugin, 225);
}
