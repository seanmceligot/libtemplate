//  tpe - a c template engine
//  Copyright (C) 2002 Sean Patrick McEligot
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#include "libtemplate/libtemplate.h"
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include "libtemplate/stringutil.h"
#include "libtemplate/debug.h"
#include "libtemplate/mybool.h"
#include "libtemplate/xmalloc.h"

static int g_verbose_debug = FALSE;
static int g_verbose_error = TRUE;
static int g_verbose_fatal = TRUE;

extern char template_regsub_prefix;
extern char *template_key_pattern;
static void usage ();

static Template *tpe;
static int g_inplace;
static char g_read_fname[FILENAME_MAX];
static char g_write_fname[FILENAME_MAX];
//FILE *out;

void myexit() {
	debug ("myexit");
	if (tpe  != NULL) {
  template_destroy (tpe);
	}
  template_shutdown ();
	xmalloc_exit();
}
static int
parse_args (int argc, char **argv)
{
  int c;
  g_inplace = FALSE;

  //init global variables
  strcpy (g_read_fname, "-");
  *g_write_fname = 0;

  for (;;) {
    static struct option long_options[] = {
      {"verbose", no_argument, NULL, 'v'},
      {"keyvalue", required_argument, NULL, 'k'},
      {"regex", required_argument, NULL, 'r'},
      {"list", required_argument, NULL, 'l'},
      {"sub-prefix", required_argument, NULL, 'p'},
      {"key-regex", required_argument, NULL, 'e'},
      {"out-file", required_argument, NULL, 'o'},
      {"in-place", no_argument, NULL, 'i'},
      {"help", no_argument, NULL, '?'},
      {NULL, 0, NULL, 0}
    };
    /* getopt_long stores the option index here. */
    int option_index = 0;
    c =
      getopt_long (argc, argv, "vk:r:l:p:e:o:i?", long_options,
                   &option_index);
    if (c == -1) {
      break;
    }
    switch (c) {
    case 0:
      /* If this option set a flag, do nothing else now. */
      if (long_options[option_index].flag != 0)
        break;
      break;
    case 'k':
      {
        char *key = optarg;
        char *value = strchr (key, '=');
        if (!value) {
          return FALSE;
        }
        *value = 0;
        value++;
        template_addkeyvalue (tpe, key, value);
      }
      break;
    case 'r':
      {
        char *substr;
        char *argument = strdup (optarg);
        char *rstr = argument;
        size_t len = strlen (rstr);
        int ok = TRUE;
        if (rstr[0] != '/') {
          ok = FALSE;
        } else if (rstr[len - 1] != '/') {
          ok = FALSE;
        } else {
          rstr[len - 1] = 0;
          rstr++;
          substr = strchr (rstr, '/');
          if (!substr) {
            ok = FALSE;
          } else {
            *substr = 0;
            substr++;
            if (!template_addregex (tpe, rstr, substr)) {
              ok = FALSE;
            }
          }
        }
        xfree (argument);
        if (!ok) {
          usage ();
          return FALSE;
        }
        break;
      }
      break;
    case 'l':
      if (!template_addstrlist (tpe, optarg)) {
        return FALSE;
      }
			
      break;
    case 'p':
      template_regsub_prefix = *optarg;
      break;
    case 'e':
      template_key_pattern = optarg;
      break;
    case 'o':
      strcpy (g_write_fname, optarg);
      break;
    case 'i':
      g_inplace = TRUE;
      break;
    case 'v':
      g_verbose_debug = TRUE;
      template_set_verbose (TRUE);
      break;
    case 'h':
      usage ();
      return FALSE;
    case '?':
      usage ();
      return FALSE;
    default:
      return FALSE;
    }                           // switch
  }                             //while

  //debugf ("argc: %d, optind %d", argc, optind);
  if (optind < argc) {
    safe_strncpy (g_read_fname, argv[optind++], FILENAME_MAX);
  }
  return TRUE;
}

void
usage ()
{
  printf ("Usage: template [OPTION] [TEMPLATE_FILE]\n"
          "Generate output from a template\n"
          "\n"
          "  -k, --keyvalue    key=value\n"
          "  -r, --regex       /regex(submatch)/substitute$1/\n"
          "  -l, --list        listname:row1_key1=val1,row1_key2=val2|row2_key1=val1,row2_key2=val2\n"
          "  -p, --sub-prefix  regex used to find keys, defaults: %c\n\n"
          "  -e, --key-regex   the $ in regex substitutions like /in(sub)in)/out$1/out/ default: %s\n\n"
          "  -o, --out-file    output file (defaults to stdout)\n"
          "  -i, --in-place    infile=outfile (backup to infile~)\n"
          "  -v, --verbose     verbose debugging\n"
          "      --help        display this help and exit\n"
          "      --version     output version information and exit\n"
          "\n"
          "With no FILE, or when FILE is -, read standard input.\n",
          template_regsub_prefix, template_key_pattern);
}
int
main (int argc, char **argv)
{
  FILE *in;
  FILE *out;
	atexit(myexit);
	xmalloc_init();
  if (!template_init ()) {
    fatal("Cannot initialize libtemplate");
    return 1;
  }
  tpe = template_new ();
  if (!parse_args (argc, argv)) {
    return 1;
  }
  if (g_inplace) {
    char *backfile;
    if (*g_write_fname != 0) {
      error ("out-file (-o) and in-place (-i) cannot be used together");
      usage ();
      abort ();
    }
    backfile = xmalloc (strlen (g_read_fname + 1));
    strcpy (backfile, g_read_fname);
    strcat (backfile, "~");
    rename (g_read_fname, backfile);
    safe_strncpy (g_write_fname, g_read_fname, FILENAME_MAX);
    safe_strncpy (g_read_fname, backfile, FILENAME_MAX);
  }
  if (*g_write_fname) {
    out = fopen (g_write_fname, "w");
    if (out == NULL) {
      fatalf ("could not create (%s)", g_write_fname);
			return 1;
    }
  } else {
    out = stdout;
  }
  debugf ("opening %s", g_read_fname);
  if (strcmp (g_read_fname, "-") == 0) {
    in = stdin;
  } else {
		debugf("opening %s", g_read_fname);
    in = fopen (g_read_fname, "rt");
    if (in == NULL) {
      fatalf ("could not open (%s)", g_read_fname);
			return 1;
    }
  }
  debugf ("parsing %s", g_read_fname);
  template_parse (tpe, in, out);
  if (in != stdin) {
					fclose (in);
	}
	if (out != stdout) {
					fclose(out);
	}
  return 0;
}

