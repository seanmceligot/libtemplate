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

extern int template_verbose;

static void usage ();

Template *tpe;

int
parse_args (int argc, char **argv, char *out_fname)
{
  int c;
  while (1) {
    static struct option long_options[] = {
      {"verbose", no_argument, &template_verbose, 1},
      {"brief", no_argument, &template_verbose, 0},
      {"keyvalue", required_argument, 0, 'k'},
      {"regex", required_argument, 0, 'r'},
      {"list", required_argument, 0, 'l'},
      {"help", no_argument, 0, '?'},
      {0, 0, 0, 0}
    };
    /* getopt_long stores the option index here. */
    int option_index = 0;
    c = getopt_long (argc, argv, "k:r:l:", long_options, &option_index);
    if (c == -1) {
      break;
    }
    switch (c) {
    case 0:
      /* If this option set a flag, do nothing else now. */
      if (long_options[option_index].flag != 0)
        break;
      debug("option %s", long_options[option_index].name);
      if (optarg)
        debug(" with arg %s", optarg);
      debug("\n");
      break;
    case 'k':
      {
        char *key = optarg;
        char *value = strchr (key, '=');
        if (!value) {
          return FALSE;
          break;
        }
        *value = 0;
        value++;
        template_addkeyvalue (tpe, key, value);
        //template_addkeyvalue(tpe,key,value); 
      }
      break;
    case 'r':
      debug("r: %s\n", optarg);
      break;
    case 'l':
      if (!template_addstrlist (tpe, optarg)) {
        return FALSE;
      }
      break;
    case 'h':
      usage ();
      return FALSE;
      break;
    case '?':
      usage ();
      return FALSE;
      break;
    default:
      return FALSE;
    }                           // switch
  }                             //while
  debug("argc: %d, optind %d\n", argc, optind);
  if (optind < argc) {
    strncpy (out_fname, argv[optind++], FILENAME_MAX - 1);
    out_fname[FILENAME_MAX] = 0;
    debug("file: %s\n", out_fname);
  } else {
    out_fname[0] = '-';
    out_fname[1] = 0;
  }
  return TRUE;
}

void
usage ()
{
  printf ("Usage: template [OPTION] [TEMPLATE_FILE]\n");
  printf ("Generate output from a template\n");
  printf ("\n");
  printf ("  -k, --keyvalue           key=value\n");
  printf ("  -r, --regex              /regex(submatch)/substitute$1/\n");
  printf
    ("  -l, --list               listname:row1_key1=val1,row1_key2=val2|row2_key1=val1,row2_key2=val2\n");
  printf ("      --help               display this help and exit\n");
  printf ("      --version            output version information and exit\n");
  printf ("\n");
  printf ("With no FILE, or when FILE is -, read standard input.\n");
}

int
main (int argc, char **argv)
{
  char out_fname[FILENAME_MAX];
  FILE *in;

  template_init ();
  tpe = template_new ();
  if (parse_args (argc, argv, out_fname)) {
    if ((out_fname[0] == 0) && (out_fname[1] == 0)) {
      in = stdin;
    } else {
      fprintf (stderr, "opening %s\n", out_fname);
      in = fopen (out_fname, "r");
      if (in == NULL) {
        fprintf (stderr, "could not open (%s)\n", out_fname);
      }
    }
    template_parse (tpe, in);
    fclose (in);
    template_destroy (tpe);
    template_shutdown ();
  }
  return 0;
}
