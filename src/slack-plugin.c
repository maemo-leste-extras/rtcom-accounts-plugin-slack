/*
 * idle-plugin.c
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

#include "config.h"

#include <glade/glade.h>
#include <glib/gi18n-lib.h>
#include <hildon/hildon.h>
#include <hildon-uri.h>
#include <libaccounts/account-plugin.h>
#include <librtcom-accounts-widgets/rtcom-account-plugin.h>
#include <librtcom-accounts-widgets/rtcom-dialog-context.h>
#include <librtcom-accounts-widgets/rtcom-edit.h>
#include <librtcom-accounts-widgets/rtcom-login.h>
#include <librtcom-accounts-widgets/rtcom-param-string.h>

#define SLACK_NEW_ACCOUNT_URI \
  "https://www.slack.com/"

typedef struct _SlackPluginClass SlackPluginClass;
typedef struct _SlackPlugin SlackPlugin;

struct _SlackPlugin
{
  RtcomAccountPlugin parent_instance;
};

struct _SlackPluginClass
{
  RtcomAccountPluginClass parent_class;
};

ACCOUNT_DEFINE_PLUGIN(SlackPlugin, slack_plugin,
                      RTCOM_TYPE_ACCOUNT_PLUGIN);

static void
slack_plugin_init(SlackPlugin *self)
{
  RtcomAccountService *service;

  RTCOM_ACCOUNT_PLUGIN(self)->name = "slack";
  RTCOM_ACCOUNT_PLUGIN(self)->capabilities =
      RTCOM_PLUGIN_CAPABILITY_ALL & ~RTCOM_PLUGIN_CAPABILITY_FORGOT_PWD;
  service = rtcom_account_plugin_add_service(RTCOM_ACCOUNT_PLUGIN(self),
                                             "haze/slack");

  g_object_set(G_OBJECT(service),
               "display-name", "Slack",
               NULL);

  glade_init();
}

static void
on_advanced_settings_response(GtkWidget *dialog, gint response,
                              RtcomDialogContext *context)
{
  if (response == GTK_RESPONSE_OK)
  {
    GError *error = NULL;
    GladeXML *xml = glade_get_widget_tree(dialog);
    GtkWidget *page = glade_xml_get_widget(xml, "page");

    if (rtcom_page_validate(RTCOM_PAGE(page), &error))
      gtk_widget_hide(dialog);
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
    gtk_widget_hide(dialog);
}

static GtkWidget *
create_advanced_settings_page(RtcomDialogContext *context)
{
  GtkWidget *dialog;

  dialog = g_object_get_data(G_OBJECT(context), "page_advanced");

  if (!dialog)
  {
    AccountItem *account;
    GtkWidget *page;
    AccountService *service;
    const gchar *profile_name;
    GtkWidget *start_page;
    gchar title[200];
    const gchar *msg;
    GladeXML *xml = glade_xml_new(PLUGIN_XML_DIR "/slack-advanced.glade",
                                  NULL, GETTEXT_PACKAGE);

    rtcom_dialog_context_take_obj(context, G_OBJECT(xml));
    dialog = glade_xml_get_widget(xml, "advanced");

    if (!dialog)
    {
      g_warning("Unable to load Advanced settings dialog");
      return dialog;
    }

    gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                           dgettext("hildon-libs", "wdgt_bd_done"),
                           GTK_RESPONSE_OK, NULL);
    account = account_edit_context_get_account(ACCOUNT_EDIT_CONTEXT(context));
    page = glade_xml_get_widget(xml, "page");
    rtcom_page_set_account(RTCOM_PAGE(page), RTCOM_ACCOUNT_ITEM(account));
    service = account_item_get_service(account);
    profile_name = account_service_get_display_name(service);
    msg = g_dgettext(GETTEXT_PACKAGE, "accountwizard_ti_advanced_settings");
    g_snprintf(title, sizeof(title), msg, profile_name);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    start_page = rtcom_dialog_context_get_start_page(context);

    if (start_page)
    {
      GtkWidget *toplevel = gtk_widget_get_toplevel(start_page);

      if (toplevel)
      {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(toplevel));
        gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
      }
    }

    g_object_ref(dialog);
    g_object_set_data_full(
          G_OBJECT(context), "page_advanced", dialog, g_object_unref);
  }

  g_signal_connect(dialog, "response",
                   G_CALLBACK(on_advanced_settings_response), context);
  g_signal_connect(dialog, "delete-event", G_CALLBACK(gtk_true), NULL);

  return dialog;
}

static void
slack_plugin_on_advanced_cb(gpointer data)
{
  GtkWidget *dialog;
  RtcomDialogContext *context = RTCOM_DIALOG_CONTEXT(data);

  dialog = create_advanced_settings_page(context);
  gtk_widget_show(dialog);
}

static void
slack_plugin_on_register_cb(gpointer userdata)
{
  gchar *uri, *lang, *p;
  GError *error = NULL;

  lang = g_strdup(getenv("LANG"));
  p = strchr(lang, '.');

  if (p)
    *p = 0;

  uri = g_strconcat(SLACK_NEW_ACCOUNT_URI, lang, NULL);
  g_free(lang);

  if (!hildon_uri_open(uri, NULL, &error))
  {
    g_warning("Failed to open browser: %s", error->message);
    g_error_free(error);
  }

  g_free(uri);
}

static void
slack_plugin_context_init(RtcomAccountPlugin *plugin,
                             RtcomDialogContext *context)
{
  gboolean editing;
  AccountItem *account;
  GtkWidget *page;
  static const gchar *invalid_chars_re = "[:'\"<>&;#\\s]";

  editing = account_edit_context_get_editing(ACCOUNT_EDIT_CONTEXT(context));
  account = account_edit_context_get_account(ACCOUNT_EDIT_CONTEXT(context));
  create_advanced_settings_page(context);

  if (editing)
  {
    page =
      g_object_new(
        RTCOM_TYPE_EDIT,
        "username-field", "account",
        "username-invalid-chars-re", invalid_chars_re,
        "items-mask", RTCOM_ACCOUNT_PLUGIN(plugin)->capabilities,
        "account", account,
        NULL);

    rtcom_edit_connect_on_advanced(
          RTCOM_EDIT(page), G_CALLBACK(slack_plugin_on_advanced_cb),
          context);
  }
  else
  {
    gchar *username_label = g_strconcat(_("accounts_fi_email"),
                                        "/",
                                        _("accounts_fi_phone"),
                                        NULL);

    page =
      g_object_new(
        RTCOM_TYPE_LOGIN,
        "username-field", "account",
        "username-label", username_label,
        "username-invalid-chars-re", invalid_chars_re,
        "items-mask", RTCOM_ACCOUNT_PLUGIN(plugin)->capabilities,
        "account", account,
        NULL);

    g_free(username_label);
    rtcom_login_connect_on_register(
      RTCOM_LOGIN(page), G_CALLBACK(slack_plugin_on_register_cb), NULL);
    rtcom_login_connect_on_advanced(
          RTCOM_LOGIN(page), G_CALLBACK(slack_plugin_on_advanced_cb),
          context);
  }

  rtcom_dialog_context_set_start_page(context, page);
}

static void
slack_plugin_class_init(SlackPluginClass *klass)
{
  RTCOM_ACCOUNT_PLUGIN_CLASS(klass)->context_init = slack_plugin_context_init;
}

