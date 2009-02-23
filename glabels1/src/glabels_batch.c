/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  glabels.c: main program module
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <config.h>

#include <unistd.h>
#include <stdlib.h>
#include <gnome.h>
#include <libgnomeprint/gnome-printer.h>

#include "merge.h"
#include "label.h"
#include "template.h"
#include "print.h"
#include "util.h"

/*============================================*/
/* Private globals                            */
/*============================================*/
static gboolean help_flag    = FALSE;
static gboolean version_flag = FALSE;
static gchar    *output      = "output.ps";
static gint     n_copies     = 1;
static gint     n_sheets     = 1;
static gboolean outline_flag = FALSE;
static gboolean reverse_flag = FALSE;

static struct poptOption options[] = {
	{"help", 'h', POPT_ARG_NONE, &help_flag, 1,
	 N_("print this message"), NULL},
	{"version", 'v', POPT_ARG_NONE, &version_flag, 0,
	 N_("print the version of glabels-batch being used"), NULL},
	{"output", 'o', POPT_ARG_STRING, &output, 0,
	 N_("set output filename (default=\"output.ps\")"), N_("filename")},
	{"sheets", 's', POPT_ARG_INT, &n_sheets, 0,
	 N_("number of sheets (default=1)"), N_("sheets")},
	{"copies", 'c', POPT_ARG_INT, &n_copies, 0,
	 N_("number of copies (default=1)"), N_("copies")},
	{"outline", 'l', POPT_ARG_NONE, &outline_flag, 0,
	 N_("print outlines (to test printer alignment)"), NULL},
	{"reverse", 'r', POPT_ARG_NONE, &reverse_flag, 0,
	 N_("print in reverse (i.e. a mirror image)"), NULL},
	{NULL, '\0', 0, NULL, 0, NULL, NULL}
};



/*****************************************************************************/
/* Main                                                                      */
/*****************************************************************************/
int
main (int argc,
      char *argv[])
{
	poptContext pctx;
	gchar **args;
	gint rc;
	GSList *p, *file_list = NULL;
	gint n_files;
	GnomePrintMaster *master = NULL;
	gchar *abs_fn;
	GnomePrinter *prn = NULL;
	glLabel *label = NULL;

	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain (PACKAGE);

	setlocale (LC_ALL, "");
	gtk_type_init ();

	/* argument parsing */
	pctx = poptGetContext (NULL, argc, argv, options, 0);
	poptSetOtherOptionHelp (pctx, _("[OPTION...] GLABELS_FILE...") );
	if ( (rc = poptGetNextOpt(pctx)) < -1 ) {
		fprintf (stderr, "%s: %s\n",
			 poptBadOption (pctx,0), poptStrerror(rc));
		poptPrintUsage (pctx, stderr, 0);
		return -1;
	}
	if ( version_flag ) {
		fprintf ( stderr, "glabels-batch %s\n", VERSION );
	}
	if ( help_flag ) {
		poptPrintHelp (pctx, stderr, 0);
		return -1;
	}
	args = (char **) poptGetArgs (pctx);
	for (n_files = 0; args && args[n_files]; n_files++) {
		file_list = g_slist_append (file_list, args[n_files]);
	}
	if ( !n_files ) {
		fprintf ( stderr, _("missing glabels file\n") );
		poptPrintHelp (pctx, stderr, 0);
		return -1;
	}
	poptFreeContext (pctx);

	/* initialize components */
	gl_merge_init ();
	gl_template_init ();

	/* now print the files */
	for (p = file_list; p; p = p->next) {
		if ( gl_label_open_xml( &label, p->data ) == LABEL_OK ) {

			if ( master == NULL ) {
				master = gnome_print_master_new ();
				abs_fn = gl_util_make_absolute ( output );
				prn = gnome_printer_new_generic_ps ( abs_fn );
				g_free( abs_fn );
				gnome_print_master_set_printer ( master, prn );
			}

			gl_print_batch( master, label, n_sheets, n_copies,
					outline_flag, reverse_flag );

			gl_label_free( &label );
		}
		else {
			fprintf ( stderr, _("cannot open glabels file %s\n"),
				  (char *)p->data );
		}
	}
	if ( master != NULL ) {
		unlink (output);
		gnome_print_master_print (master);
	}

	g_slist_free (file_list);

	return 0;
}
