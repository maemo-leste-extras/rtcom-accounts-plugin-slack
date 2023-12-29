/*
 * advanced-page.h
 *
 * Copyright (C) 2022 Ivaylo Dimitrov <ivo.g.dimitrov.75@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef __ADVANCED_PAGE_H_INCLUDED__
#define __ADVANCED_PAGE_H_INCLUDED__

static void
set_widget_setting (gpointer key, gpointer value, gpointer userdata)
{
  GtkWidget *widget = key;

  if (RTCOM_IS_PARAM_INT(widget))
  {
    rtcom_param_int_set_value(RTCOM_PARAM_INT(widget),
                              GPOINTER_TO_INT(value));
  }
  else if (GTK_IS_ENTRY(widget))
    gtk_entry_set_text(GTK_ENTRY(widget), value);
  else if (HILDON_IS_CHECK_BUTTON(widget))
  {
    hildon_check_button_set_active(HILDON_CHECK_BUTTON(widget),
                                   GPOINTER_TO_INT(value));
  }
  else
  {
    g_warning("%s: unhandled widget type %s (%s)", G_STRFUNC,
              g_type_name(G_TYPE_FROM_INSTANCE(widget)),
              gtk_widget_get_name(widget));
  }
}

static void
get_advanced_settings (GtkWidget *widget, GHashTable *advanced_settings)
{
  const gchar *name = gtk_widget_get_name(widget);

  if (RTCOM_IS_PARAM_INT(widget))
  {
    gint value = rtcom_param_int_get_value(RTCOM_PARAM_INT(widget));
    g_hash_table_replace(advanced_settings, widget,
                         GINT_TO_POINTER(value));
  }
  else if (GTK_IS_ENTRY(widget))
  {
    gchar *data = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
    g_object_set_data_full(G_OBJECT(widget), "adv_data", data, g_free);
    g_hash_table_replace(advanced_settings, widget, data);
  }
  else if (HILDON_IS_CHECK_BUTTON(widget))
  {
    gboolean active =
        hildon_check_button_get_active(HILDON_CHECK_BUTTON(widget));

    g_hash_table_replace(advanced_settings, widget, GINT_TO_POINTER(active));
  }
  else if (GTK_IS_CONTAINER(widget))
  {
    gtk_container_foreach(GTK_CONTAINER(widget),
                          (GtkCallback)get_advanced_settings,
                          advanced_settings);
  }
  else if (!GTK_IS_LABEL(widget))
  {
    g_warning("%s: unhandled widget type %s (%s)", G_STRFUNC,
              g_type_name(G_TYPE_FROM_INSTANCE(widget)), name);
  }
}

static void
on_advanced_settings_response(GtkWidget *dialog, gint response,
                              RtcomDialogContext *context)
{
  GHashTable *advanced_settings =
      g_object_get_data(G_OBJECT(context), "settings");

  if (response == GTK_RESPONSE_OK)
  {
    GError *error = NULL;
    GladeXML *xml = glade_get_widget_tree(dialog);
    GtkWidget *page = glade_xml_get_widget(xml, "page");

    if (rtcom_page_validate(RTCOM_PAGE(page), &error))
    {
      get_advanced_settings(dialog, advanced_settings);
      gtk_widget_hide(dialog);
    }
    else
    {
      g_warning("advanced page validation failed");

      if (error)
      {
        g_warning("%s: error \"%s\"", G_STRFUNC, error->message);
        hildon_banner_show_information(dialog, NULL, error->message);
        g_error_free(error);
      }
    }
  }
  else
  {
    g_hash_table_foreach(advanced_settings, set_widget_setting, dialog);
    gtk_widget_hide(dialog);
  }
}

#endif /* __ADVANCED_PAGE_H_INCLUDED__ */
