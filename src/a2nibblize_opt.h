#ifndef UUID_89649336eeac4af084f7f089c7048476
#define UUID_89649336eeac4af084f7f089c7048476

#include <stdint.h>

struct opts_t
  {
    int test;
    uint8_t volume;
  };

extern struct opts_t *parse_opts(int argc, char *argv[]);

#endif
