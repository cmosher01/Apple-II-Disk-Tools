#ifndef UUID_e4c1853235df4bef809f233965122e73
#define UUID_e4c1853235df4bef809f233965122e73

#include <stdint.h>

struct opts_t
  {
    int test;
    int hex;
    uint_fast16_t dos_version;
    uint8_t catalog_track;
    uint_fast8_t used_sectors;
    uint8_t volume;
  };

extern struct opts_t *parse_opts(int argc, char *argv[]);

#endif
