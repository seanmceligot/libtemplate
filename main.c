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

void
template_usage (char *argv0)
{
  printf ("usage %s filename", argv0);
  exit (1);
}

int
main (int argc, char **argv)
{
  int arg = 1;
  char *file;
  Template *tpe;
  FILE *in;
  if (argc < 2)
    {
      template_usage (argv[0]);
    }
  //argc = 4;
  //gchar** argv = g_strsplit("./tpe name=Test variables:name=Foo,type=int|name=bar,type=long test.tpe", " ", 4);
  template_init ();
  tpe = template_new ();
  file = argv[argc - 1];
  for (arg = 1; arg < argc - 1; arg++)
    {
      gchar *str = argv[arg];
      template_addstrparam (tpe, str);
    }

  fprintf (stderr, "opening %s\n", file);
  in = fopen (file, "r");
  if (in == NULL)
    {
      fprintf (stderr, "could not open (%s)\n", file);
    }
  template_parse (tpe, in);
  template_destroy (tpe);
  fclose (in);
  template_shutdown ();
  //g_strfreev(argv);
  return 0;
}
