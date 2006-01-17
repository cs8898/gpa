/* encryptdlg.c  -  The GNU Privacy Assistant
 *	Copyright (C) 2000, 2001 G-N-U GmbH.
 *      Copyright (C) 2002, 2003 Miguel Coca.
 *
 * This file is part of GPA
 *
 * GPA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "gpa.h"
#include "gtktools.h"
#include "gpawidgets.h"
#include "gpakeyselector.h"
#include "gpapastrings.h"
#include "encryptdlg.h"

/* Internal functions */
static void changed_select_row_cb (GtkTreeSelection *treeselection,
				   gpointer user_data);
static void toggle_sign_cb (GtkToggleButton *togglebutton, gpointer user_data);


/* Properties */
enum
{
  PROP_0,
  PROP_WINDOW,
};

static GObjectClass *parent_class = NULL;

static void
gpa_file_encrypt_dialog_get_property (GObject     *object,
				      guint        prop_id,
				      GValue      *value,
				      GParamSpec  *pspec)
{
  GpaFileEncryptDialog *dialog = GPA_FILE_ENCRYPT_DIALOG (object);
  
  switch (prop_id)
    {
    case PROP_WINDOW:
      g_value_set_object (value,
			  gtk_window_get_transient_for (GTK_WINDOW (dialog)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gpa_file_encrypt_dialog_set_property (GObject     *object,
				      guint        prop_id,
				      const GValue      *value,
				      GParamSpec  *pspec)
{
  GpaFileEncryptDialog *dialog = GPA_FILE_ENCRYPT_DIALOG (object);

  switch (prop_id)
    {
    case PROP_WINDOW:
      gtk_window_set_transient_for (GTK_WINDOW (dialog),
				    g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gpa_file_encrypt_dialog_finalize (GObject *object)
{  
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpa_file_encrypt_dialog_class_init (GpaFileEncryptDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  parent_class = g_type_class_peek_parent (klass);
  
  object_class->finalize = gpa_file_encrypt_dialog_finalize;
  object_class->set_property = gpa_file_encrypt_dialog_set_property;
  object_class->get_property = gpa_file_encrypt_dialog_get_property;

  /* Properties */
  g_object_class_install_property (object_class,
				   PROP_WINDOW,
				   g_param_spec_object 
				   ("window", "Parent window",
				    "Parent window", GTK_TYPE_WIDGET,
				    G_PARAM_WRITABLE|G_PARAM_CONSTRUCT_ONLY));
}

static void
gpa_file_encrypt_dialog_init (GpaFileEncryptDialog *dialog)
{
  GtkAccelGroup *accelGroup;
  GtkWidget *vboxEncrypt;
  GtkWidget *labelKeys;
  GtkWidget *scrollerKeys;
  GtkWidget *clistKeys;
  GtkWidget *checkerSign;
  GtkWidget *checkerArmor;
  GtkWidget *labelWho;
  GtkWidget *scrollerWho;
  GtkWidget *clistWho;

  /* Set up the dialog */
  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
			  GTK_STOCK_OK, GTK_RESPONSE_OK,
			  _("_Cancel"), GTK_RESPONSE_CANCEL, NULL);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Encrypt files"));
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, 
				     FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);

  accelGroup = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (dialog), accelGroup);

  vboxEncrypt = GTK_DIALOG (dialog)->vbox;
  gtk_container_set_border_width (GTK_CONTAINER (vboxEncrypt), 5);

  labelKeys = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (labelKeys), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vboxEncrypt), labelKeys, FALSE, FALSE, 0);

  scrollerKeys = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW (scrollerKeys),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vboxEncrypt), scrollerKeys, TRUE, TRUE, 0);
  gtk_widget_set_usize (scrollerKeys, 350, 120);

  clistKeys = gpa_key_selector_new (FALSE);
  g_signal_connect (G_OBJECT (gtk_tree_view_get_selection 
			      (GTK_TREE_VIEW (clistKeys))),
		    "changed", G_CALLBACK (changed_select_row_cb),
		    dialog);
  dialog->clist_keys = clistKeys;
  gtk_container_add (GTK_CONTAINER (scrollerKeys), clistKeys);
  gpa_connect_by_accelerator (GTK_LABEL (labelKeys), clistKeys, accelGroup,
			      _("_Public Keys"));

 
  checkerSign = gpa_check_button_new (accelGroup, _("_Sign"));
  gtk_box_pack_start (GTK_BOX (vboxEncrypt), checkerSign, FALSE, FALSE, 0);
  dialog->check_sign = checkerSign;
  gtk_signal_connect (GTK_OBJECT (checkerSign), "toggled",
		      GTK_SIGNAL_FUNC (toggle_sign_cb), dialog);

  labelWho = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (labelWho), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vboxEncrypt), labelWho, FALSE, TRUE, 0);

  scrollerWho = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW (scrollerWho),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
  gtk_widget_set_usize (scrollerWho, 350, 75);
  gtk_box_pack_start (GTK_BOX (vboxEncrypt), scrollerWho, TRUE, TRUE, 0);

  clistWho =  gpa_key_selector_new (TRUE);
  dialog->clist_who = clistWho;
  gtk_container_add (GTK_CONTAINER (scrollerWho), clistWho);
  gpa_connect_by_accelerator (GTK_LABEL (labelWho), clistWho, accelGroup,
			      _("Sign _as "));
  gtk_widget_set_sensitive (clistWho, FALSE);

  checkerArmor = gpa_check_button_new (accelGroup, _("A_rmor"));
  gtk_box_pack_start (GTK_BOX (vboxEncrypt), checkerArmor, FALSE, FALSE, 0);
  dialog->check_armor = checkerArmor;

}

GType
gpa_file_encrypt_dialog_get_type (void)
{
  static GType encrypt_dialog_type = 0;
  
  if (!encrypt_dialog_type)
    {
      static const GTypeInfo encrypt_dialog_info =
	{
	  sizeof (GpaFileEncryptDialogClass),
	  (GBaseInitFunc) NULL,
	  (GBaseFinalizeFunc) NULL,
	  (GClassInitFunc) gpa_file_encrypt_dialog_class_init,
	  NULL,           /* class_finalize */
	  NULL,           /* class_data */
	  sizeof (GpaFileEncryptDialog),
	  0,              /* n_preallocs */
	  (GInstanceInitFunc) gpa_file_encrypt_dialog_init,
	};
      
      encrypt_dialog_type = g_type_register_static (GTK_TYPE_DIALOG,
						    "GpaFileEncryptDialog",
						    &encrypt_dialog_info, 0);
    }
  
  return encrypt_dialog_type;
}

/* API */

GtkWidget *gpa_file_encrypt_dialog_new (GtkWidget *parent)
{
  GpaFileEncryptDialog *dialog;
  
  dialog = g_object_new (GPA_FILE_ENCRYPT_DIALOG_TYPE,
			 "window", parent,
			 NULL);

  return GTK_WIDGET(dialog);
}

GList *gpa_file_encrypt_dialog_recipients (GpaFileEncryptDialog *dialog)
{
  return gpa_key_selector_get_selected_keys (GPA_KEY_SELECTOR (dialog->clist_keys));
}

gboolean gpa_file_encrypt_dialog_sign (GpaFileEncryptDialog *dialog)
{
  return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->check_sign));
}

GList *gpa_file_encrypt_dialog_signers (GpaFileEncryptDialog *dialog)
{
  return gpa_key_selector_get_selected_keys (GPA_KEY_SELECTOR (dialog->clist_who));
}

gboolean gpa_file_encrypt_dialog_get_armor (GpaFileEncryptDialog *dialog)
{
  return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->check_armor));
}

static void
changed_select_row_cb (GtkTreeSelection *treeselection, gpointer user_data)
{
  GpaFileEncryptDialog *dialog = user_data;
  
  if (gpa_key_selector_has_selection (GPA_KEY_SELECTOR (dialog->clist_keys)))
    {
      gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
					 GTK_RESPONSE_OK, TRUE);
    }
  else
    {
      gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
					 GTK_RESPONSE_OK, FALSE);
    }
}

static void
toggle_sign_cb (GtkToggleButton *togglebutton, gpointer user_data)
{
  GpaFileEncryptDialog *dialog = user_data;
  gtk_widget_set_sensitive (dialog->clist_who,
                            gtk_toggle_button_get_active (togglebutton));
}

