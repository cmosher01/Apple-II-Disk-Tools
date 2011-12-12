#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <minmax.h>
#include <binary-io.h>

#include "ctest.h"
#include "a2const.h"
#include "a2catalog_opt.h"
#include "nibblize_4_4.h"
#include "nibblize_5_3.h"
#include "nibblize_5_3_alt.h"
#include "nibblize_6_2.h"



/*
 * Writes an 8-bit byte at the given pointer.
 *
 * b  the byte to write out
 * pp address of pointer to write b at; incremented on exit
 */
static void b_out(uint8_t b, uint8_t **pp)
{
  assert(pp);
  *(*pp)++ = b;
}

static void test_b_out(ctest_ctx *ctx)
{
  const uint8_t SENTINEL = UINT8_C(0xFD);
  uint8_t image[3];
  const uint8_t TAG = UINT8_C(0xa5);
  const uint8_t b = TAG;
  uint8_t *p = image+1;

  image[0] = SENTINEL;
  image[1] = SENTINEL;
  image[2] = SENTINEL;

  b_out(b,&p);

  CTEST(ctx,image[1]==TAG);
  CTEST(ctx,image[0]==SENTINEL);
  CTEST(ctx,image[2]==SENTINEL);
  CTEST(ctx,p==image+2);
}



/*
 * Writes a 16-bit word, low-order byte first, at the given pointer.
 *
 * w  the word to write out (upper 16 bits are ignored)
 * pp address of pointer to write w at; incremented on exit
 */
static void w_out(uint16_t w, uint8_t **pp)
{
  assert(pp);
  b_out((uint8_t)w,pp);
  w >>= 8;
  b_out((uint8_t)w,pp);
}

static void test_w_out(ctest_ctx *ctx)
{
  const uint8_t SENTINEL = UINT8_C(0xFD);
  const uint8_t TAG_HI = UINT8_C(0xA5);
  const uint8_t TAG_LO = UINT8_C(0x5A);
  const uint16_t TAG = UINT16_C(0xA55A);
  const uint16_t w = TAG;
  uint8_t image[4];
  uint8_t *p = image+1;

  image[0] = SENTINEL;
  image[1] = SENTINEL;
  image[2] = SENTINEL;
  image[3] = SENTINEL;

  w_out(w,&p);

  CTEST(ctx,image[1]==TAG_LO);
  CTEST(ctx,image[2]==TAG_HI);
  CTEST(ctx,image[0]==SENTINEL);
  CTEST(ctx,image[3]==SENTINEL);
  CTEST(ctx,p==image+3);
}



static void map_out(uint16_t m, uint8_t **pp)
{
  assert(pp);
  b_out((uint8_t)(m >> 8),pp);
  b_out((uint8_t)m,pp);
  w_out(UINT16_C(0),pp);
}



/*
 * Writes n 8-bit bytes at the given pointer.
 *
 * n  the number of times to write b
 * b  the byte to write out
 * pp address of pointer to start writing b's at; incremented on exit
 */
static void n_b_out(uint_fast32_t n, uint8_t b, uint8_t **pp)
{
  assert(pp);
  while (n--)
    {
      b_out(b,pp);
    }
}

static void test_n_b_out(ctest_ctx *ctx)
{
  const uint8_t SENTINEL = 0xFD;
  const uint8_t TAG = 0xA5;
  const uint8_t b = TAG;
  uint8_t image[5];
  uint8_t *p = image+1;
  const uint_fast32_t N = 3;

  image[0] = SENTINEL;
  image[1] = SENTINEL;
  image[2] = SENTINEL;
  image[3] = SENTINEL;
  image[4] = SENTINEL;

  n_b_out(N,b,&p);

  CTEST(ctx,image[1]==TAG);
  CTEST(ctx,image[2]==TAG);
  CTEST(ctx,image[3]==TAG);
  CTEST(ctx,image[0]==SENTINEL);
  CTEST(ctx,image[4]==SENTINEL);
  CTEST(ctx,p==image+1+N);
}






static uint_fast8_t sectors_per_track(uint_fast16_t version)
{
  assert(version/100 == 3);
  return (uint_fast8_t)(version < 330 ? UINT8_C(13) : UINT8_C(16));
}

static void test_sectors_per_track(ctest_ctx *ctx)
{
  int cs;

  cs = sectors_per_track(310);
  CTEST(ctx,cs==13);
  cs = sectors_per_track(320);
  CTEST(ctx,cs==13);
  cs = sectors_per_track(321);
  CTEST(ctx,cs==13);
  cs = sectors_per_track(330);
  CTEST(ctx,cs==16);
  cs = sectors_per_track(331);
  CTEST(ctx,cs==16);
  cs = sectors_per_track(332);
  CTEST(ctx,cs==16);
}



static uint16_t get_free_track_map(uint_fast16_t version)
{
  uint_fast8_t cs = sectors_per_track(version);
  uint16_t mk = 0;

  while (cs--)
    {
      mk >>= 1;
      mk |= UINT16_C(0x8000);
    }
  return mk;
}

static void test_get_free_track_map(ctest_ctx *ctx)
{
  uint16_t fr;

  fr  = get_free_track_map(310);
  CTEST(ctx,fr==0xFFF8);
  fr = get_free_track_map(331);
  CTEST(ctx,fr==0xFFFF);
}



static void allocate_n_sectors(uint_fast8_t c_used_sectors, uint16_t *track_map)
{
	int uint16_value;

  assert(track_map);
	uint16_value = *track_map << c_used_sectors;
	*track_map = (uint16_t)(uint16_value & UINT16_C(0xFFFF));
}

/* TODO need to verify this on a real disk */
static void test_allocate_n_sectors(ctest_ctx *ctx)
{
  uint16_t bitmap = get_free_track_map(310);

  allocate_n_sectors(3,&bitmap);
  /* 13 free sectors, minus 3 free sectors, equals 10 free sectors */
  CTEST(ctx,bitmap==0xFFC0);

  bitmap = get_free_track_map(330);
  allocate_n_sectors(3,&bitmap);
  /* 16 free sectors, minus 3 free sectors, equals 13 free sectors */
  CTEST(ctx,bitmap==0xFFF8);
}



/*
 * Writes a catalog-sector link at the given pointer.
 *
 * sect  sector number, or zero to mean "no link"
 * track catalog track number
 * pp address of pointer to start writing at; incremented on exit
 */
static void sector_link_out(uint8_t sect, uint8_t track, uint8_t **pp)
{
  assert(pp);
  b_out(0,pp);
  if (sect) {
	  b_out(track,pp);
  } else {
	  b_out(0,pp);
  }
  b_out(sect,pp);
}

static void test_sector_link_out(ctest_ctx *ctx)
{
  const uint8_t SENTINEL = 0xFD;
  const uint8_t SECTOR = 0x0F;
  const uint8_t TRACK = 0x11;
  const uint8_t NO_LINK = 0;
  uint8_t image[5];
  uint8_t *p = image+1;

  image[0] = SENTINEL;
  image[1] = SENTINEL;
  image[2] = SENTINEL;
  image[3] = SENTINEL;
  image[4] = SENTINEL;

  sector_link_out(SECTOR,TRACK,&p);

  CTEST(ctx,image[1]==0);
  CTEST(ctx,image[2]==TRACK);
  CTEST(ctx,image[3]==SECTOR);
  CTEST(ctx,image[0]==SENTINEL);
  CTEST(ctx,image[4]==SENTINEL);
  CTEST(ctx,p==image+4);



  p = image+1;

  image[0] = SENTINEL;
  image[1] = SENTINEL;
  image[2] = SENTINEL;
  image[3] = SENTINEL;
  image[4] = SENTINEL;

  sector_link_out(NO_LINK,TRACK,&p);

  CTEST(ctx,image[1]==NO_LINK);
  CTEST(ctx,image[2]==NO_LINK);
  CTEST(ctx,image[3]==NO_LINK);
  CTEST(ctx,image[0]==SENTINEL);
  CTEST(ctx,image[4]==SENTINEL);
  CTEST(ctx,p==image+4);
}

static void catalog_sector_out(uint8_t sector_number, uint8_t track, uint8_t **pp)
{
  assert(pp);
  /*
   * link to previous sector, except for last sector in
   * the sequence (sector 1) which has no link.
   */
  sector_link_out((uint8_t)(sector_number-1),track,pp);
  /* rest of sector is zeroes */
  n_b_out(BYTES_PER_SECTOR-3,0,pp);
}



static void volume_sector_map_out(uint_fast16_t version, uint8_t catalog_track, uint_fast8_t used, uint8_t **pp)
{
  uint8_t tr;

  assert(pp);
  for (tr = 0; tr < TRACKS_PER_DISK; ++tr)
    {
      uint16_t bitmap = get_free_track_map(version);
      if (tr==catalog_track)
        {
          /* catalog track is always fully allocated */
          allocate_n_sectors(sectors_per_track(version),&bitmap);
        }
      else
        {
          const uint_fast8_t u = (uint_fast8_t)MIN(used,sectors_per_track(version));
          allocate_n_sectors(u,&bitmap);
          used = (uint_fast8_t)(used-u);
        }
      map_out(bitmap,pp);
    }
}

/* B.A.D. p. 4-9 */
#define TS_OFFSET_IN_FILEMAP UINT8_C(0x0C)

/*
 * Maximum track/sector pairs in one sector
 * of a file's sector map.
 */
static uint8_t get_max_ts_in_filemap(void)
{
  return (BYTES_PER_SECTOR-TS_OFFSET_IN_FILEMAP)/UINT8_C(2);
}

static uint8_t calc_dos_version_byte(uint_fast16_t version) {
  assert(version/100 == 3);
	return (uint8_t)(version/10%10);
}

static void catalog_VTOC_out(uint_fast16_t version, uint8_t catalog_track, uint_fast8_t used, uint8_t volume, uint8_t **pp)
{
  assert(pp);

  sector_link_out((uint8_t)(sectors_per_track(version)-1),catalog_track,pp);

  b_out(calc_dos_version_byte(version),pp);
  n_b_out(2,0,pp);

  b_out(volume,pp);
  n_b_out(0x20,0,pp);
  b_out(get_max_ts_in_filemap(),pp);
  n_b_out(8,0,pp);

  b_out(catalog_track,pp);
  b_out(1,pp);
  n_b_out(2,0,pp);

  b_out(TRACKS_PER_DISK,pp);
  b_out(sectors_per_track(version),pp);
  w_out(BYTES_PER_SECTOR,pp);

  volume_sector_map_out(version,catalog_track,used,pp);

  n_b_out(60,0,pp);
}


static uint8_t default_vtoc[0x100] =
{
  0x00, 0x11, 0x0f, 0x03, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x11, 0x01, 0x00, 0x00, 0x23, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xe0, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
  0xff, 0xff
};

static void test_catalog_VTOC_out(ctest_ctx *ctx)
{
  uint8_t computed_vtoc[0x100];
  uint8_t *p = computed_vtoc;
  uint_fast16_t i;

  catalog_VTOC_out(330,0x11,0x25,254,&p);

  for (i = 0; i < BYTES_PER_SECTOR; ++i)
    {
      CTEST(ctx,computed_vtoc[i]==default_vtoc[i]);
    }
}

static void catalog_track_out(uint_fast16_t version, uint8_t catalog_track, uint_fast8_t used, uint8_t volume, uint8_t **pp)
{
  uint8_t sc;

  assert(pp);
  catalog_VTOC_out(version,catalog_track,used,volume,pp);
  for (sc = 1; sc < sectors_per_track(version); ++sc)
    {
      catalog_sector_out(sc,catalog_track,pp);
    }
}




static void put_buffer_hex(uint8_t *p, uint_fast32_t c)
{
  uint_fast32_t i = 0;

  while (i<c)
    {
      ++i;
      printf("%02x",*p++);
      if (!(i%0x10))
        {
          printf("\n");
        }
    }
}

static void put_buffer_raw(uint8_t *p, uint_fast32_t c)
{
  uint_fast32_t i = 0;

  SET_BINARY(1);
  while (i<c)
    {
      ++i;
      putchar(*p++);
    }
}


static size_t calc_buf_size(uint_fast16_t dos_version) {
	const uint_fast16_t bpt = (uint_fast16_t)(sectors_per_track(dos_version)*BYTES_PER_SECTOR);

	return bpt*sizeof(uint8_t);
}

static int run_program(struct opts_t *opts)
{
  uint8_t *t;
  uint8_t *x;
	const size_t c = calc_buf_size(opts->dos_version);

  x = t = (uint8_t*)malloc(c);

  catalog_track_out(opts->dos_version,opts->catalog_track,opts->used_sectors,opts->volume,&x);

  assert(x == t+c);

  if (opts->hex)
    {
      put_buffer_hex(t,c);
    }
  else
    {
      put_buffer_raw(t,c);
    }

  free(t);

  return EXIT_SUCCESS;
}

static int run_tests(void)
{
  int r;

  ctest_ctx *ctx = ctest_ctx_alloc();

  printf("running unit tests...\n");
  test_b_out(ctx);
  test_w_out(ctx);
  test_n_b_out(ctx);
  test_sectors_per_track(ctx);
  test_get_free_track_map(ctx);
  test_allocate_n_sectors(ctx);
  test_sector_link_out(ctx);
  test_catalog_VTOC_out(ctx);

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
