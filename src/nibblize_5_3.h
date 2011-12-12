#ifndef UUID_d901a32e0e3e4805a76264474fb28d05
#define UUID_d901a32e0e3e4805a76264474fb28d05

#include <stdint.h>
#include "ctest.h"

void nibblize_5_3_encode(const uint8_t **original, uint8_t **encoded);
void nibblize_5_3_decode(const uint8_t **original, uint8_t **decoded);

void test_nibblize_5_3(ctest_ctx *ctx);

#endif
