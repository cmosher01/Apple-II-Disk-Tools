#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "nibblize_4_4.h"

#include <stdint.h>

#include "ctest.h"



uint16_t nibblize_4_4_encode(uint8_t n)
{
  /*
    hgfedcba00000000   n<<8
    000000000hgfedcb   n>>1
  | 1010101010101010   0xAAAA
  ------------------
    1g1e1c1a1h1f1d1b
  */
  return (n << 8) | (n >> 1) | 0xAAAA;
}

uint8_t nibblize_4_4_decode(uint16_t n)
{
  /*
    g1e1c1a1h1f1d1b1   n<<1|1
  & 000000001g1e1c1a   n>>8
    ------------------
    00000000hgfedcba
  */
  return ((n << 1) | 1) & (n >> 8);
}



static const struct
  {
    uint8_t unencoded_byte;
    uint16_t nibblized_byte;
  } test_values[] =
{
  {0x00,0xAAAA},
  {0x01,0xABAA},
  {0x02,0xAAAB},
  {0x03,0xABAB},
  {0x04,0xAEAA},
  {0x05,0xAFAA},
  {0x06,0xAEAB},
  {0x07,0xAFAB},
  {0x08,0xAAAE},
  {0x09,0xABAE},
  {0x0A,0xAAAF},
  {0x0B,0xABAF},
  {0x0C,0xAEAE},
  {0x0D,0xAFAE},
  {0x0E,0xAEAF},
  {0x0F,0xAFAF},
  {0xFE,0xFEFF}
};

void test_nibblize_4_4(ctest_ctx *ctx)
{
  const int c = sizeof(test_values)/sizeof(test_values[0]);
  int i;
  for (i = 0; i < c; ++i)
    {
      CTEST(ctx,nibblize_4_4_encode(test_values[i].unencoded_byte)==test_values[i].nibblized_byte);
      CTEST(ctx,nibblize_4_4_decode(test_values[i].nibblized_byte)==test_values[i].unencoded_byte);
    }
}
