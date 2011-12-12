#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <binary-io.h>

#include "ctest.h"
#include "a2const.h"
#include "a2nibblize_opt.h"
#include "nibblize_4_4.h"
#include "nibblize_5_3.h"
#include "nibblize_5_3_alt.h"
#include "nibblize_6_2.h"


static void put_buffer(uint8_t *p, int c)
{
  SET_BINARY(1);
  while (c--)
    {
      putchar(*p++);
    }
}

static int get_buffer(uint8_t *p, int c)
{
  int oc = c;
  int b;
  SET_BINARY(0);
  while ((b = getchar()) != EOF)
    {
      if (!c--)
        {
          fprintf(stderr,"too many hexadecimal bytes in input (expected at most 0x%X)\n",oc);
          exit(1);
        }
      *p++ = b;
    }
  return c;
}





static void b_out(uint8_t b, uint8_t **pp)
{
  *(*pp)++ = b;
}

static void w_out(uint16_t w, uint8_t **pp)
{
  b_out(w,pp);
  w >>= 8;
  b_out(w,pp);
}

static void n_b_out(uint_fast32_t n, uint8_t b, uint8_t **pp)
{
  while (n--)
    {
      b_out(b,pp);
    }
}




typedef uint8_t sector_t[BYTES_PER_SECTOR];

#define SECTORS_PER_TRACK_16 0x10
typedef sector_t track16_t[SECTORS_PER_TRACK_16];
typedef track16_t disk16_t[TRACKS_PER_DISK];
#define DISK16_SIZE (BYTES_PER_SECTOR*SECTORS_PER_TRACK_16*TRACKS_PER_DISK)
#define NIB16_SIZE (TRACKS_PER_DISK*(0x30+SECTORS_PER_TRACK_16*(6+3+(4*2)+3+3+343+3+0x1B)+0x110))

#define SECTORS_PER_TRACK_13 0xD
typedef sector_t track13_t[SECTORS_PER_TRACK_13];
typedef track13_t disk13_t[TRACKS_PER_DISK];
#define DISK13_SIZE (BYTES_PER_SECTOR*SECTORS_PER_TRACK_13*TRACKS_PER_DISK)
#define NIB13_SIZE (TRACKS_PER_DISK*(0x30+SECTORS_PER_TRACK_13*(6+3+(4*2)+3+3+411+3+0x1B)+0x240))

#define DIFF_SIZE (DISK16_SIZE-DISK13_SIZE)



static uint8_t mp_sector13[SECTORS_PER_TRACK_13];
#define SKEW_13_SECTORS 0xA

static void build_mp_sector13(void)
{
  uint8_t s = 0;
  uint_fast8_t i;
  for (i = 0; i < SECTORS_PER_TRACK_13; ++i)
    {
      mp_sector13[i] = s;
      s += SKEW_13_SECTORS;
      s %= SECTORS_PER_TRACK_13;
    }
}








static void data_marker_out(uint8_t **pp)
{
  b_out(0xD5,pp);
  b_out(0xAA,pp);
  b_out(0xAD,pp);
}

static void addr_out(uint8_t volume, uint8_t track, uint8_t sector, uint8_t **pp)
{
  w_out(nibblize_4_4_encode(volume),pp);
  w_out(nibblize_4_4_encode(track),pp);
  w_out(nibblize_4_4_encode(sector),pp);
  w_out(nibblize_4_4_encode(volume^track^sector),pp);
}

static void end_marker_out(uint8_t **pp)
{
  b_out(0xDE,pp);
  b_out(0xAA,pp);
  b_out(0xEB,pp);
}







static void addr13out(uint8_t volume, uint8_t track, uint8_t sector, uint8_t **pp)
{
  b_out(0xD5,pp);
  b_out(0xAA,pp);
  b_out(0xB5,pp);
  addr_out(volume,track,sector,pp);
  end_marker_out(pp);
}

static void data13out(const uint8_t *data, int alt, uint8_t **pp)
{
  data_marker_out(pp);
  if (alt)
    {
      nibblize_5_3_alt_encode(&data, pp);
    }
  else
    {
      nibblize_5_3_encode(&data, pp);
    }
  end_marker_out(pp);
}

static void sect13out(uint8_t *data, uint8_t volume, uint8_t track, uint8_t sector, uint8_t **pp)
{
  addr13out(volume, track, sector, pp);
  n_b_out(6, 0xFF, pp);
  data13out(data, track == 0 && sector == 0, pp);
  n_b_out(0x1B, 0xFF, pp);
}

static void write13nib(uint8_t volume, disk13_t image, uint8_t **pp)
{
  uint8_t t;
  uint8_t s;
  for (t = 0; t < TRACKS_PER_DISK; ++t)
    {
      n_b_out(0x30, 0xFF, pp);
      for (s = 0; s < SECTORS_PER_TRACK_13; ++s)
        {
          sect13out(image[t][mp_sector13[s]], volume, t, mp_sector13[s], pp);
        }
      n_b_out(0x240, 0xFF, pp);
    }
}









static void addr16out(uint8_t volume, uint8_t track, uint8_t sector, uint8_t **pp)
{
  b_out(0xD5,pp);
  b_out(0xAA,pp);
  b_out(0x96,pp);
  addr_out(volume,track,sector,pp);
  end_marker_out(pp);
}

static void data16out(const uint8_t *data, uint8_t **pp)
{
  data_marker_out(pp);
  nibblize_6_2_encode(&data, pp);
  end_marker_out(pp);
}

static void sect16out(uint8_t *data, uint8_t volume, uint8_t track, uint8_t sector, uint8_t **pp)
{
  addr16out(volume, track, sector, pp);
  n_b_out(6, 0xFF, pp);
  data16out(data, pp);
  n_b_out(0x1B, 0xFF, pp);
}

static const uint8_t mp_sector16[] = { 0x0, 0x7, 0xE, 0x6, 0xD, 0x5, 0xC, 0x4, 0xB, 0x3, 0xA, 0x2, 0x9, 0x1, 0x8, 0xF };

static void write16nib(uint8_t volume, disk16_t image, uint8_t **pp)
{
  uint8_t t;
  uint8_t s;
  for (t = 0; t < TRACKS_PER_DISK; ++t)
    {
      n_b_out(0x30, 0xFF, pp);
      for (s = 0; s < SECTORS_PER_TRACK_16; ++s)
        {
          sect16out(image[t][mp_sector16[s]], volume, t, s, pp);
        }
      n_b_out(0x110, 0xFF, pp);
    }
}

static int run_program(struct opts_t *opts)
{
  int c_remain;
  uint8_t *image = (uint8_t*)malloc(DISK16_SIZE);
  c_remain = get_buffer(image,DISK16_SIZE);
  if (!c_remain)
    {
      uint8_t *pnib = (uint8_t*)malloc(NIB16_SIZE);
      uint8_t *onib = pnib;
      write16nib(opts->volume,*((disk16_t*)image),&pnib);
      pnib = onib;
      put_buffer(pnib,NIB16_SIZE);
      free(pnib);
    }
  else if (c_remain==DIFF_SIZE)
    {
      uint8_t *pnib = (uint8_t*)malloc(NIB13_SIZE);
      uint8_t *onib = pnib;
      build_mp_sector13();
      write13nib(opts->volume,*((disk13_t*)image),&pnib);
      pnib = onib;
      put_buffer(pnib,NIB13_SIZE);
      free(pnib);
    }
  else
    {
      fprintf(stderr,"wrong number of hexadecimal bytes in input (expected 0x%X or 0x%X)\n",DISK16_SIZE,DISK13_SIZE);
      exit(1);
    }
  free(image);
  return EXIT_SUCCESS;
}

static int run_tests(void)
{
  int r;

  ctest_ctx *ctx = ctest_ctx_alloc();

  printf("running unit tests...\n");

  test_nibblize_4_4(ctx);
  test_nibblize_5_3(ctx);
  test_nibblize_5_3_alt(ctx);
  test_nibblize_6_2(ctx);

  r = ctest_count_fail(ctx) ? EXIT_FAILURE : EXIT_SUCCESS;

  ctest_ctx_free(ctx);

  return r;
}

int main(int argc, char *argv[])
{
  struct opts_t *opts = parse_opts(argc,argv);

  if (opts->test)
    {
      return run_tests();
    }

  return run_program(opts);
}
