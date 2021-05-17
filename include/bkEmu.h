#ifndef __BKEMU_H
#define __BKEMU_H

#include "stdint.h"
#include "BkScreen.h"
#include "defines.h"

//using namespace bk;

extern pdp_regs pdp;

int32_t bk_loop();
void bk_reset();

#endif
