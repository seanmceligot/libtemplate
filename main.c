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
#include "libtemplate.h"
#include <getopt.h>
#include "stringutil.h"
#include "libtemplate_util.h"

extern int template_verbose;
extern char template_regsub_prefix;
extern char *template_key_pattern;
static void usage ();

static Template *tpe;
static int g_inplace;
static char g_read_fname[FILENAME_MAX];
static char g_write_fname[FILENAME_MAX];
FILE *out;

static int
parse_args (int argc, char **argv)
{
  int c;
  g_inplace = FALSE;

  //init global variables
  strcpy (g_read_fname, "-");
  *g_write_fname = 0;

  for (;;)
    {
      static struct option long_options[] = {
	{"verbose", no_argument, &template_verbose, 1},
	{"brief", no_argument, &template_verbose, 0},
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
	getopt_long (argc, argv, "k:r:l:p:e:o:i?", long_options,
		     &option_index);
      if (c == -1)
	{
	  break;
	}
      switch (c)
	{
	case 0:
	  /* If this option set a flag, do nothing else now. */
	  if (long_options[option_index].flag != 0)
	    break;
	  debug ("option %s", long_options[option_index].name);
	  if (optarg)
	    debug (" with arg %s", optarg);
	  debug ("\n");
	  break;
	case 'k':
	  {
	    char *key = optarg;
	    char *value = strchr (key, '=');
	    if (!value)
	      {
		return FALSE;
	      }
	    *value = 0;
	    value++;
	    template_addkeyvalue (tpe, key, value);
	    //template_addkeyvalue(tpe,key,value); 
	  }
	  break;
	case 'r':
	  {
	    char *rstr = optarg;
	    size_t len = strlen (rstr);
	    if (rstr[0] != '/')
	      {
		usage ();
		return FALSE;
	      }
	    if (rstr[len - 1] != '/')
	      {
		usage ();
		return FALSE;
	      }
	    rstr++;
	    {
	      gchar **v_pair = g_strsplit (rstr, "/", 2);
	      if (!v_pair[1])
		{
		  usage ();
		  g_strfreev (v_pair);
		  return FALSE;
		}
	      {
		char *regstr = v_pair[0];
		char *substr = v_pair[1];
		debug ("replace %s with %s\n", regstr, substr);
		if (!template_addregex (tpe, regstr, substr))
		  {
		    g_strfreev (v_pair);
		    return FALSE;
		  }
		g_strfreev (v_pair);
	      }
	    }
	  }
	  break;
	case 'l':
	  if (!template_addstrlist (tpe, optarg))
	    {
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
	case 'h':
	  usage ();
	  return FALSE;
	case '?':
	  usage ();
	  return FALSE;
	default:
	  return FALSE;
	}			// switch
    }				//while
  //debug ("argc: %d, optind %d\n", argc, optind);
  if (optind < argc)
    {
      safe_strncpy (g_read_fname, argv[optind++], FILENAME_MAX);
      debug ("infile: %s\n", g_read_fname);
    }
  return TRUE;
}

void
usage ()
{
  printf ("\
Usage: template [OPTION] [TEMPLATE_FILE]\n\
Generate output from a template\n\
\n\
  -k, --keyvalue    key=value\n\
  -r, --regex       /regex(submatch)/substitute$1/\n\
  -l, --list        listname:row1_key1=val1,row1_key2=val2|row2_key1=val1,row2_key2=val2\n\
  -p, --sub-prefix  regex used to find keys, defaults: %c\n
  -e, --key-regex   the $ in regex substitutions like /in(sub)in)/out$1/out/ default: %s\n
  -o, --out-file    output file (defaults to stdout)\n\
  -i, --in-place    infile=outfile (backup to infile~)\n\
  -v, --verbose     verbose debugging\n\
      --help        display this help and exit\n\
      --version     output version information and exit\n\
\n\
With no FILE, or when FILE is -, read standard input.\n\
", template_regsub_prefix, template_key_pattern);
}

int
main (int argc, char **argv)
{
  FILE *in;
  FILE *out;

  if (!template_init ())
    {
      fprintf (stderr, "Cannot initialize libtemplate\n");
      abort ();
    }
  tpe = template_new ();
  if (!parse_args (argc, argv))
    {
      return 1;
    }
  if (g_inplace)
    {
      char *backfile;
      if (*g_write_fname != 0)
	{
	  error ("out-file (-o) and in-place (-i) cannot be used together\n");
	  usage ();
	  abort ();
	}
      backfile = malloc (strlen (g_read_fname + 1));
      strcpy (backfile, g_read_fname);
      strcat (backfile, "~");
      debug ("renaming %s->%s\n", g_read_fname, backfile);
      rename (g_read_fname, backfile);
      safe_strncpy (g_write_fname, g_read_fname, FILENAME_MAX);
      safe_strncpy (g_read_fname, backfile, FILENAME_MAX);
    }
  if (strlen (g_write_fname))
    {
      out = fopen (g_write_fname, "w");
      if (out == NULL)
	{
	  fatal ("could not create (%s)\n", g_write_fname);
	}
    }
  else
    {
      out = stdout;
    }
  if (strcmp (g_read_fname, "-") == 0)
    {
      in = stdin;
    }
  else
    {
      debug ("opening %s\n", g_read_fname);
      in = fopen (g_read_fname, "r");
      if (in == NULL)
	{
	  fatal ("could not open (%s)\n", g_read_fname);
	}
    }
  template_parse (tpe, in, out);
  fclose (in);
  template_destroy (tpe);
  template_shutdown ();
  return 0;
}
