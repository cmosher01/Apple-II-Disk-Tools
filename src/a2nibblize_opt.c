#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "a2nibblize_opt.h"

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "a2const.h"

static const char shortopts[] = "hTVv:";

static const struct option longopts[] =
  {
    {"help",no_argument,0,'h'},
    {"test",no_argument,0,'T'},
    {"version",no_argument,0,'V'},
    {"volume",required_argument,0,'v'},
    {0,0,0,0}
  };

static void help(int argc, char *argv[])
{
  (void)argc;
  printf("Converts an Apple ][ floppy disk image from .do format\n");
  printf("to .nib \"nibble\" format.  The input disk must be a logical\n");
  printf("13 or 16 sector, DOS order, floppy disk image.\n");
  printf("\n");
  printf("Usage: %s [OPTION...]\n",argv[0]);
  printf("Options:\n");
  printf("  -h, --help           shows this help\n");
  printf("  -T, --test           runs all unit tests\n");
  printf("  -V, --version        shows version information\n");
  printf("  -v, --volume=VOLUME  \"DISK VOLUME\" to use, default 254\n");
}

static struct opts_t *opts_factory(void)
  {
    struct opts_t *opts = (struct opts_t*)malloc(sizeof(struct opts_t));

    opts->test = 0;
    opts->volume = 254; /* as in "DISK VOLUME 254" */

    return opts;
  }

static void version(void)
{
  printf("%s\n",PACKAGE_STRING);
  printf("\n");
  printf("%s\n","Copyright (C) 2011, by Chris Mosher.");
  printf("%s\n","License GPLv3+: GNU GPL version 3 or later <http://www.gnu.org/licenses/gpl.html>.");
  printf("%s\n","This is free software: you are free to change and redistribute it.");
  printf("%s\n","There is NO WARRANTY, to the extent permitted by law.");
}

static long get_num_optarg(void)
{
  return strtol(optarg,0,0);
}

struct opts_t *parse_opts(int argc, char *argv[])
  {
    int c;

    struct opts_t *opts = opts_factory();

    while ((c = getopt_long(argc,argv,shortopts,longopts,0)) >= 0)
      {
        switch (c)
          {
          case 'T':
            opts->test = 1;
            break;
          case 'V':
            version();
            exit(0);
            break;
          case 'v':
            opts->volume = get_num_optarg();
            break;
          case 0:
            break;
          case 'h':
          default:
            help(argc,argv);
            exit(0);
          }
      }

    return opts;
  }
