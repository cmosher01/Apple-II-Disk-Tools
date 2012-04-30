#ifndef UUID_6881609590944a84af78213e0cc16550
#define UUID_6881609590944a84af78213e0cc16550

#include <stdint.h>
#include "ctest/ctest.h"

extern uint16_t nibblize_4_4_encode(uint8_t n);
extern uint8_t nibblize_4_4_decode(uint16_t n);

extern void test_nibblize_4_4(ctest_ctx *ctx);

#endif
